#include "mog/Constants.h"
#include "mog/core/AudioPlayer.h"
#include "mog/core/AudioPlayerNative.h"
#include <future>

using namespace mog;

#pragma - AudioChannel

AudioChannel::AudioChannel(AudioPlayer *audioPlayer) {
    this->audioChannelNative = unique_ptr<AudioChannelNative>(new AudioChannelNative(audioPlayer->audioPlayerNative.get()));
}

void AudioChannel::play(string filename, bool cache) {
    if (this->mute) return;
    this->audioChannelNative->load(filename.c_str(), cache);
    this->audioChannelNative->play();
}

void AudioChannel::pause() {
    if (this->mute) return;
    this->audioChannelNative->pause();
}

void AudioChannel::resume() {
    if (this->mute) return;
    this->audioChannelNative->resume();
}

void AudioChannel::stop() {
    this->audioChannelNative->stop();
}

void AudioChannel::close() {
    this->audioChannelNative->close();
}

void AudioChannel::seek(float offset) {
    this->audioChannelNative->seek(offset);
}

void AudioChannel::setLoop(bool isLoop) {
    this->audioChannelNative->setLoop(isLoop);
}

bool AudioChannel::isLoop() {
    return this->audioChannelNative->isLoop();
}

void AudioChannel::setVolume(float volume) {
    this->audioChannelNative->setVolume(volume);
}

float AudioChannel::getVolume() {
    return this->audioChannelNative->getVolume();
}

void AudioChannel::setMute(bool mute) {
    this->mute = mute;
    if (mute) {
        this->stop();
    }
}

bool AudioChannel::isMute() {
    return this->mute;
}

bool AudioChannel::isLoaded() {
    return this->audioChannelNative->isLoaded();
}

AudioChannel::State AudioChannel::getState() {
    return this->audioChannelNative->getState();
}

void AudioChannel::execute() {
    
}

#pragma - AudioPlayer

AudioPlayer *AudioPlayer::instance;

AudioPlayer::AudioPlayer() {
    this->audioPlayerNative = unique_ptr<AudioPlayerNative>(new AudioPlayerNative());
    auto channel = shared_ptr<AudioChannel>(new AudioChannel(this));
    this->poolOneShotChannels.emplace_back(channel);
}

void AudioPlayer::initialize() {
    if (AudioPlayer::instance == nullptr) {
        AudioPlayer::instance = new AudioPlayer();
    }
}

shared_ptr<AudioChannel> AudioPlayer::createChannel(string key) {
    if (AudioPlayer::instance->channels[key]) {
        return AudioPlayer::instance->channels[key];
    }
    auto channel = shared_ptr<AudioChannel>(new AudioChannel(AudioPlayer::instance));
    AudioPlayer::instance->channels[key] = channel;
    return channel;
}

shared_ptr<AudioChannel> AudioPlayer::getChannel(string key) {
    return AudioPlayer::instance->channels[key];
}

void AudioPlayer::removeChannel(string key) {
    auto channel = AudioPlayer::instance->channels[key];
    if (channel) {
        channel->close();
        AudioPlayer::instance->channels.erase(key);
    }
}

void AudioPlayer::preloadOne(string filename) {
    AudioPlayer::instance->audioPlayerNative->preload(filename.c_str());
}

void AudioPlayer::playOneShot(string filename) {
    shared_ptr<AudioChannel> channel = nullptr;
    for (int i = (int)AudioPlayer::instance->poolOneShotChannels.size() - 1; i >= 0; i--) {
        const shared_ptr<AudioChannel> &c = AudioPlayer::instance->poolOneShotChannels[i];
        if (c->getState() == AudioChannel::State::Playing) {
            continue;
        }
        
        if (c->getState() == AudioChannel::State::Paused) {
            c->stop();
        }
        if (channel) {
            c->close();
            AudioPlayer::instance->poolOneShotChannels.erase(AudioPlayer::instance->poolOneShotChannels.begin() + i);
        } else {
            channel = c;
        }
    }
    if (!channel) {
        channel = shared_ptr<AudioChannel>(new AudioChannel(AudioPlayer::instance));
        AudioPlayer::instance->poolOneShotChannels.emplace_back(channel);
    }
    channel->play(filename);
}

void AudioPlayer::onPause() {
    for (const auto &pair : AudioPlayer::instance->channels) {
        if (pair.second->getState() == AudioChannel::State::Playing) {
            AudioPlayer::instance->resumeChannels.emplace_back(pair.second);
            pair.second->pause();
        }
    }
    for (const auto &c : AudioPlayer::instance->poolOneShotChannels) {
        if (c->getState() == AudioChannel::State::Playing) {
            AudioPlayer::instance->resumeChannels.emplace_back(c);
            c->pause();
        }
    }
}

void AudioPlayer::onResume() {
    for (const auto &c : AudioPlayer::instance->resumeChannels) {
        c->resume();
    }
    AudioPlayer::instance->resumeChannels.clear();
}

void AudioPlayer::execute() {
    for (const auto &pair : AudioPlayer::instance->channels) {
        pair.second->execute();
    }
}
