#pragma once

#include <atomic>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include <Analysis/AnalysisSnapshot.h>

//==============================================================================
/** Capture → offline analyze (**tasks T010/T011**, constitution §II realtime).

    **`processBlock`** appends PCM only while armed (**Recording**) — gated by frozen timing mode (**spec timing**) —
    never allocates / analyzes / logs heavily on the audio thread.
*/
class PluginProcessor final : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    /** Nominal contiguous-duration guard (**spec FR‑016**, research/plan): rejects ultra-short taps before analyzers run. */
    static constexpr double kMinQualifyingRecordedSeconds { 1.0 };

    static constexpr float kTempoClampMinBpm { 48.f };
    static constexpr int kMaxCaptureBars { 32 };
    static constexpr int kBeatsPerBarMvp { 4 };

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    loopbreaker::AnalysisPublishSlot& getAnalysisPublishSlot() noexcept { return publishSlot; }

    /** Analyze button (**Recording** arm): freezes **`TimingMode`** / banner; resets capture write offset on audio thread. */
    void armRecordingFromUi();

    /** Stop button: seals buffer on audio thread then runs extractor/analyzer/suggestions on message thread. */
    void sealRecordingFromUi();

private:
    [[nodiscard]] bool queryTransportPlaying() const noexcept;

    void freezeTimingModeForNextArmLocked();

    void resizeCaptureBuffers (double sampleRate);

    void scheduleSealAndAnalyze (size_t numSamplesCaptured, uint32_t generationSnapshot);

    void runCapturedAnalyze (size_t numSamplesCaptured, uint32_t generationSnapshot);

    loopbreaker::AnalysisPublishSlot publishSlot;

    double currentSampleRate { 44100.0 };

    std::vector<float> captureL;
    std::vector<float> captureR;
    size_t captureCapacitySamples {};

    /** Written only from **`processBlock`** (single producer). */
    size_t captureWritePos {};

    std::atomic<bool> pendingArm { false };
    std::atomic<bool> sealRequested { false };
    std::atomic<bool> captureArmed { false };

    /** Mirrors **`TimingMode`** gate chosen under **`publishSlot.mutex`** at arm — audio thread reads without locking. */
    std::atomic<bool> armedUsesTransportPlayingGate { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
