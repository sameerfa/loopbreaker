#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <span>

#include <juce_events/juce_events.h>

#include <Analysis/AudioFeatureExtractor.h>
#include <Analysis/LoopAnalyzer.h>
#include <Analysis/SuggestionEngine.h>

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
#if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
                         .withInput ("Input", juce::AudioChannelSet::stereo(), true)
    #endif
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
{
}

PluginProcessor::~PluginProcessor() {}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::resizeCaptureBuffers (double sampleRate)
{
    currentSampleRate = sampleRate;

    const auto beatsTotal = static_cast<double> (kMaxCaptureBars * kBeatsPerBarMvp);
    const auto secondsMax = beatsTotal * (60.0 / static_cast<double> (kTempoClampMinBpm));

    captureCapacitySamples = static_cast<size_t> (std::ceil (secondsMax * sampleRate)) + 64;
    captureL.assign (captureCapacitySamples, 0.f);
    captureR.assign (captureCapacitySamples, 0.f);
    captureWritePos = 0;
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    resizeCaptureBuffers (sampleRate);

    pendingArm.store (false, std::memory_order_release);
    sealRequested.store (false, std::memory_order_release);
    captureArmed.store (false, std::memory_order_release);
}

void PluginProcessor::releaseResources() {}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

bool PluginProcessor::queryTransportPlaying() const noexcept
{
    if (auto* ph = getPlayHead())
    {
        const auto optPos = ph->getPosition();

        if (optPos.hasValue())
            return optPos->getIsPlaying();
    }

    return false;
}

void PluginProcessor::freezeTimingModeForNextArmLocked()
{
    armedUsesTransportPlayingGate.store (false, std::memory_order_relaxed);

    if (auto* ph = getPlayHead())
    {
        const auto optPos = ph->getPosition();

        if (optPos.hasValue())
        {
            publishSlot.snapshot.timingMode = loopbreaker::TimingMode::HostAligned;
            publishSlot.snapshot.fallbackBannerLiteral.clear();
            armedUsesTransportPlayingGate.store (true, std::memory_order_release);
            return;
        }
    }

    publishSlot.snapshot.timingMode = loopbreaker::TimingMode::CapturedDurationFallback;
    publishSlot.snapshot.syncFallbackBannerFromMode();
}

void PluginProcessor::armRecordingFromUi()
{
    std::scoped_lock lk (publishSlot.mutex);

    publishSlot.analyzeGeneration.fetch_add (1u, std::memory_order_relaxed);
    publishSlot.snapshot.analysisState = loopbreaker::AnalysisState::Recording;
    publishSlot.snapshot.errorText.clear();
    publishSlot.snapshot.metrics = {};
    publishSlot.snapshot.suggestions = {};

    freezeTimingModeForNextArmLocked();
    pendingArm.store (true, std::memory_order_release);
}

void PluginProcessor::sealRecordingFromUi()
{
    sealRequested.store (true, std::memory_order_release);
}

int PluginProcessor::getRequestedLoopBars() const noexcept
{
    return requestedLoopBars.load (std::memory_order_relaxed);
}

void PluginProcessor::setRequestedLoopBarsFromUi (int bars)
{
    if (bars == 8 || bars == 16 || bars == kMaxCaptureBars)
        requestedLoopBars.store (bars, std::memory_order_relaxed);
}

void PluginProcessor::scheduleSealAndAnalyze (size_t numSamplesCaptured, uint32_t generationSnapshot)
{
    juce::MessageManager::callAsync ([this, numSamplesCaptured, generationSnapshot]
                                     { runCapturedAnalyze (numSamplesCaptured, generationSnapshot); });
}

void PluginProcessor::runCapturedAnalyze (size_t numSamplesCaptured, uint32_t generationSnapshot)
{
    if (publishSlot.analyzeGeneration.load (std::memory_order_acquire) != generationSnapshot)
        return;

    {
        std::scoped_lock lk (publishSlot.mutex);

        if (publishSlot.analyzeGeneration.load (std::memory_order_relaxed) != generationSnapshot)
            return;

        publishSlot.snapshot.analysisState = loopbreaker::AnalysisState::Analyzing;
    }

    const auto minSamples = static_cast<size_t> (
        std::ceil (currentSampleRate * kMinQualifyingRecordedSeconds));

    if (numSamplesCaptured < minSamples)
    {
        std::scoped_lock lk (publishSlot.mutex);

        if (publishSlot.analyzeGeneration.load (std::memory_order_relaxed) != generationSnapshot)
            return;

        publishSlot.snapshot.analysisState = loopbreaker::AnalysisState::Error;
        publishSlot.snapshot.errorText = "Recording too short—capture at least ~1 s of audio.";
        publishSlot.snapshot.metrics = {};
        publishSlot.snapshot.suggestions = {};
        return;
    }

    if (captureL.empty() || captureR.empty() || numSamplesCaptured > captureL.size())
    {
        std::scoped_lock lk (publishSlot.mutex);

        if (publishSlot.analyzeGeneration.load (std::memory_order_relaxed) != generationSnapshot)
            return;

        publishSlot.snapshot.analysisState = loopbreaker::AnalysisState::Error;
        publishSlot.snapshot.errorText = "Capture buffer unavailable.";
        return;
    }

    loopbreaker::AudioFeatureExtractor extractor;
    extractor.prepare (currentSampleRate);

    auto frames = extractor.run (captureL.data(),
                                 captureR.data(),
                                 static_cast<int> (numSamplesCaptured),
                                 currentSampleRate);

    auto metrics = loopbreaker::LoopAnalyzer::analyze (
        std::span<const loopbreaker::FeatureFrame> (frames.data(), frames.size()));

    auto deck = loopbreaker::SuggestionEngine::run (metrics);

    std::scoped_lock lk (publishSlot.mutex);

    if (publishSlot.analyzeGeneration.load (std::memory_order_relaxed) != generationSnapshot)
        return;

    publishSlot.snapshot.metrics = metrics;
    publishSlot.snapshot.suggestions = deck;
    publishSlot.snapshot.analysisState = loopbreaker::AnalysisState::Complete;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (static_cast<int> (i), 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();

    if (pendingArm.exchange (false, std::memory_order_acq_rel))
    {
        captureWritePos = 0;
        captureArmed.store (true, std::memory_order_release);
    }

    if (sealRequested.exchange (false, std::memory_order_acq_rel))
    {
        if (captureArmed.exchange (false, std::memory_order_acq_rel))
        {
            const auto sealedFrames = captureWritePos;
            const auto genSnap = publishSlot.analyzeGeneration.load (std::memory_order_acquire);

            scheduleSealAndAnalyze (sealedFrames, genSnap);
        }
    }

    const auto transportPlaying = queryTransportPlaying();

    const bool gateOpen = captureArmed.load (std::memory_order_acquire)
                          && ((! armedUsesTransportPlayingGate.load (std::memory_order_acquire))
                              || transportPlaying);

    if (gateOpen && captureCapacitySamples > 0 && ! captureL.empty() && ! captureR.empty())
    {
        const auto srcCh = buffer.getNumChannels();
        const float* srcL = buffer.getReadPointer (0);
        const float* srcR = srcCh > 1 ? buffer.getReadPointer (1) : srcL;

        auto space = captureCapacitySamples - captureWritePos;

        if (space > 0)
        {
            const auto toCopy = static_cast<size_t> (
                std::min (n, static_cast<int> (space)));

            std::memcpy (captureL.data() + captureWritePos, srcL, toCopy * sizeof (float));
            std::memcpy (captureR.data() + captureWritePos, srcR, toCopy * sizeof (float));

            captureWritePos += toCopy;
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
