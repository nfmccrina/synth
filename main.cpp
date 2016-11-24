/*
 *  This extra small demo sends a random samples to your speakers.
 */

#include <alsa/asoundlib.h>
#include <string>
#include <cstdint>
#include <stdexcept>

enum class PCMSign { SIGNED, UNSIGNED };
enum PCMSampleSize { SS_LOW = 8, SS_MEDIUM = 16, SS_HIGH = 24 };
enum PCMChannelFormat { MONO = 1, STEREO = 2};
enum PCMSampleRate { SR_LOW = 11025, SR_MEDIUM = 44100, SR_HIGH = 96000 };

struct PCMFormat
{
    PCMSign sign;
    PCMSampleSize sampleSize;
    PCMChannelFormat channelFormat;
    PCMSampleRate sampleRate;
};

struct PCMData
{
    PCMFormat format;
    uint32_t frames;
    uint8_t* pcm_data;
};

class ALSAInterop
{
public:
    snd_pcm_format_t GetALSAFormat(PCMFormat& format);
};

class PCMOpenException : public std::runtime_error
{
public:
    PCMOpenException(const std::string& arg);
    PCMOpenException(const char* arg);
};

class PCMWriteException : public std::runtime_error
{
public:
    PCMWriteException(const std::string& arg);
    PCMWriteException(const char* arg);
};

class PCMFormatException : public std::runtime_error
{
public:
    PCMFormatException(const std::string& arg);
    PCMFormatException(const char* arg);
};

class ALSAPCMHandle
{
public:
    ALSAPCMHandle();
    ALSAPCMHandle(std::string deviceName);
    ~ALSAPCMHandle();
    void SetParams(PCMFormat& format);
    void Writei(PCMData& data);

private:
    snd_pcm_t* native_handle;
};

int main()
{
    ALSAPCMHandle handle;
    PCMFormat format;

    format.sign = PCMSign::UNSIGNED;
    format.sampleSize = PCMSampleSize::SS_LOW;
    format.channelFormat = PCMChannelFormat::MONO;
    format.sampleRate = PCMSampleRate::SR_LOW;

    handle.SetParams(format);

    PCMData data;

    data.format = format;
    data.frames = 22050;
    data.pcm_data = new uint8_t[data.frames * format.sampleSize];

    for (int i = 0; i < (data.frames * format.sampleSize); i++)
    {
        data.pcm_data[i] = random() & 0xff;
    }

    handle.Writei(data);

    return 0;
}

PCMOpenException::PCMOpenException(const std::string& arg) :
    std::runtime_error(arg)
{
}

PCMOpenException::PCMOpenException(const char* arg) :
    std::runtime_error(arg)
{
}

PCMWriteException::PCMWriteException(const std::string& arg) :
    std::runtime_error(arg)
{
}

PCMWriteException::PCMWriteException(const char* arg) :
    std::runtime_error(arg)
{
}

PCMFormatException::PCMFormatException(const std::string& arg) :
    std::runtime_error(arg)
{
}

PCMFormatException::PCMFormatException(const char* arg) :
    std::runtime_error(arg)
{
}

ALSAPCMHandle::ALSAPCMHandle()
{
    int err;
    err = snd_pcm_open(&(this->native_handle), "default",
        SND_PCM_STREAM_PLAYBACK, 0);

    if (err < 0)
    {
        throw PCMOpenException("Playback open error");
    }
}

ALSAPCMHandle::ALSAPCMHandle(std::string deviceName)
{
    int err;
    err = snd_pcm_open(&(this->native_handle), deviceName.c_str(),
        SND_PCM_STREAM_PLAYBACK, 0);

    if (err < 0)
    {
        throw PCMOpenException("Playback open error");
    }
}

ALSAPCMHandle::~ALSAPCMHandle()
{
    snd_pcm_close(this->native_handle);
}

void ALSAPCMHandle::SetParams(PCMFormat& format)
{
    ALSAInterop interop;
    int err;

    err = snd_pcm_set_params(this->native_handle,
        interop.GetALSAFormat(format), SND_PCM_ACCESS_RW_INTERLEAVED,
        format.channelFormat, format.sampleRate, 1, 500000);

    if (err < 0)
    {
        throw PCMFormatException(snd_strerror(err));
    }
}

void ALSAPCMHandle::Writei(PCMData& data)
{
    int err;

    err = snd_pcm_writei(this->native_handle, data.pcm_data,
        data.frames * (data.format.sampleSize * data.format.channelFormat));

    if (err < 0)
    {
        err = snd_pcm_recover(this->native_handle, err, 1);

        if (err < 0)
        {
            throw PCMWriteException(snd_strerror(err));
        }
    }
    else if (err > 0 && err < data.frames)
    {
        throw PCMWriteException("Did not write all frames");
    }
}

snd_pcm_format_t ALSAInterop::GetALSAFormat(PCMFormat& format)
{
    snd_pcm_format_t result = SND_PCM_FORMAT_UNKNOWN;

    if (format.sign == PCMSign::UNSIGNED)
    {
        if (format.sampleSize == PCMSampleSize::SS_LOW)
        {
            result = SND_PCM_FORMAT_U8;
        }
    }
    else
    {
        if (format.sampleSize == PCMSampleSize::SS_LOW)
        {
            result = SND_PCM_FORMAT_S8;
        }
    }

    return result;
}

