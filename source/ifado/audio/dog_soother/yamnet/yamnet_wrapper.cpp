#include "yamnet_wrapper.h"
#undef LOG_TAG
#include "log.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#define LOG_TAG "yamnet_wrapper.cpp"
#include <chrono>
#include <cmath>
#include <complex>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>

// 存储AudioSet 类别标签
const std::vector<std::string> class_names = {"Speech",
                                              "Child speech, kid speaking",
                                              "Conversation",
                                              "Narration, monologue",
                                              "Babbling",
                                              "Speech synthesizer",
                                              "Shout",
                                              "Bellow",
                                              "Whoop",
                                              "Yell",
                                              "Children shouting",
                                              "Screaming",
                                              "Whispering",
                                              "Laughter",
                                              "Baby laughter",
                                              "Giggle",
                                              "Snicker",
                                              "Belly laugh",
                                              "Chuckle, chortle",
                                              "Crying, sobbing",
                                              "Baby cry, infant cry",
                                              "Whimper",
                                              "Wail, moan",
                                              "Sigh",
                                              "Singing",
                                              "Choir",
                                              "Yodeling",
                                              "Chant",
                                              "Mantra",
                                              "Child singing",
                                              "Synthetic singing",
                                              "Rapping",
                                              "Humming",
                                              "Groan",
                                              "Grunt",
                                              "Whistling",
                                              "Breathing",
                                              "Wheeze",
                                              "Snoring",
                                              "Gasp",
                                              "Pant",
                                              "Snort",
                                              "Cough",
                                              "Throat clearing",
                                              "Sneeze",
                                              "Sniff",
                                              "Run",
                                              "Shuffle",
                                              "Walk, footsteps",
                                              "Chewing, mastication",
                                              "Biting",
                                              "Gargling",
                                              "Stomach rumble",
                                              "Burping, eructation",
                                              "Hiccup",
                                              "Fart",
                                              "Hands",
                                              "Finger snapping",
                                              "Clapping",
                                              "Heart sounds, heartbeat",
                                              "Heart murmur",
                                              "Cheering",
                                              "Applause",
                                              "Chatter",
                                              "Crowd",
                                              "Hubbub, speech noise, speech babble",
                                              "Children playing",
                                              "Animal",
                                              "Domestic animals, pets",
                                              "Dog",  // 狗
                                              "Bark", // 狗
                                              "Yip",
                                              "Howl", // 狗
                                              "Bow-wow",
                                              "Growling",      // 狗
                                              "Whimper (dog)", // 狗
                                              "Cat",           // 猫
                                              "Purr",          // 猫
                                              "Meow",          // 猫
                                              "Hiss",          // 猫
                                              "Caterwaul",     // 猫
                                              "Livestock, farm animals, working animals",
                                              "Horse",
                                              "Clip-clop",
                                              "Neigh, whinny",
                                              "Cattle, bovinae",
                                              "Moo",
                                              "Cowbell",
                                              "Pig",
                                              "Oink",
                                              "Goat",
                                              "Bleat",
                                              "Sheep",
                                              "Fowl",
                                              "Chicken, rooster",
                                              "Cluck",
                                              "Crowing, cock-a-doodle-doo",
                                              "Turkey",
                                              "Gobble",
                                              "Duck",
                                              "Quack",
                                              "Goose",
                                              "Honk",
                                              "Wild animals",
                                              "Roaring cats (lions, tigers)",
                                              "Roar",
                                              "Bird",
                                              "Bird vocalization, bird call, bird song",
                                              "Chirp, tweet",
                                              "Squawk",
                                              "Pigeon, dove",
                                              "Coo",
                                              "Crow",
                                              "Caw",
                                              "Owl",
                                              "Hoot",
                                              "Bird flight, flapping wings",
                                              "Canidae, dogs, wolves",
                                              "Rodents, rats, mice",
                                              "Mouse",
                                              "Patter",
                                              "Insect",
                                              "Cricket",
                                              "Mosquito",
                                              "Fly, housefly",
                                              "Buzz",
                                              "Bee, wasp, etc.",
                                              "Frog",
                                              "Croak",
                                              "Snake",
                                              "Rattle",
                                              "Whale vocalization",
                                              "Music",
                                              "Musical instrument",
                                              "Plucked string instrument",
                                              "Guitar",
                                              "Electric guitar",
                                              "Bass guitar",
                                              "Acoustic guitar",
                                              "Steel guitar, slide guitar",
                                              "Tapping (guitar technique)",
                                              "Strum",
                                              "Banjo",
                                              "Sitar",
                                              "Mandolin",
                                              "Zither",
                                              "Ukulele",
                                              "Keyboard (musical)",
                                              "Piano",
                                              "Electric piano",
                                              "Organ",
                                              "Electronic organ",
                                              "Hammond organ",
                                              "Synthesizer",
                                              "Sampler",
                                              "Harpsichord",
                                              "Percussion",
                                              "Drum kit",
                                              "Drum machine",
                                              "Drum",
                                              "Snare drum",
                                              "Rimshot",
                                              "Drum roll",
                                              "Bass drum",
                                              "Timpani",
                                              "Tabla",
                                              "Cymbal",
                                              "Hi-hat",
                                              "Wood block",
                                              "Tambourine",
                                              "Rattle (instrument)",
                                              "Maraca",
                                              "Gong",
                                              "Tubular bells",
                                              "Mallet percussion",
                                              "Marimba, xylophone",
                                              "Glockenspiel",
                                              "Vibraphone",
                                              "Steelpan",
                                              "Orchestra",
                                              "Brass instrument",
                                              "French horn",
                                              "Trumpet",
                                              "Trombone",
                                              "Bowed string instrument",
                                              "String section",
                                              "Violin, fiddle",
                                              "Pizzicato",
                                              "Cello",
                                              "Double bass",
                                              "Wind instrument, woodwind instrument",
                                              "Flute",
                                              "Saxophone",
                                              "Clarinet",
                                              "Harp",
                                              "Bell",
                                              "Church bell",
                                              "Jingle bell",
                                              "Bicycle bell",
                                              "Tuning fork",
                                              "Chime",
                                              "Wind chime",
                                              "Change ringing (campanology)",
                                              "Harmonica",
                                              "Accordion",
                                              "Bagpipes",
                                              "Didgeridoo",
                                              "Shofar",
                                              "Theremin",
                                              "Singing bowl",
                                              "Scratching (performance technique)",
                                              "Pop music",
                                              "Hip hop music",
                                              "Beatboxing",
                                              "Rock music",
                                              "Heavy metal",
                                              "Punk rock",
                                              "Grunge",
                                              "Progressive rock",
                                              "Rock and roll",
                                              "Psychedelic rock",
                                              "Rhythm and blues",
                                              "Soul music",
                                              "Reggae",
                                              "Country",
                                              "Swing music",
                                              "Bluegrass",
                                              "Funk",
                                              "Folk music",
                                              "Middle Eastern music",
                                              "Jazz",
                                              "Disco",
                                              "Classical music",
                                              "Opera",
                                              "Electronic music",
                                              "House music",
                                              "Techno",
                                              "Dubstep",
                                              "Drum and bass",
                                              "Electronica",
                                              "Electronic dance music",
                                              "Ambient music",
                                              "Trance music",
                                              "Music of Latin America",
                                              "Salsa music",
                                              "Flamenco",
                                              "Blues",
                                              "Music for children",
                                              "New-age music",
                                              "Vocal music",
                                              "A capella",
                                              "Music of Africa",
                                              "Afrobeat",
                                              "Christian music",
                                              "Gospel music",
                                              "Music of Asia",
                                              "Carnatic music",
                                              "Music of Bollywood",
                                              "Ska",
                                              "Traditional music",
                                              "Independent music",
                                              "Song",
                                              "Background music",
                                              "Theme music",
                                              "Jingle (music)",
                                              "Soundtrack music",
                                              "Lullaby",
                                              "Video game music",
                                              "Christmas music",
                                              "Dance music",
                                              "Wedding music",
                                              "Happy music",
                                              "Sad music",
                                              "Tender music",
                                              "Exciting music",
                                              "Angry music",
                                              "Scary music",
                                              "Wind",
                                              "Rustling leaves",
                                              "Wind noise (microphone)",
                                              "Thunderstorm",
                                              "Thunder",
                                              "Water",
                                              "Rain",
                                              "Raindrop",
                                              "Rain on surface",
                                              "Stream",
                                              "Waterfall",
                                              "Ocean",
                                              "Waves, surf",
                                              "Steam",
                                              "Gurgling",
                                              "Fire",
                                              "Crackle",
                                              "Vehicle",
                                              "Boat, Water vehicle",
                                              "Sailboat, sailing ship",
                                              "Rowboat, canoe, kayak",
                                              "Motorboat, speedboat",
                                              "Ship",
                                              "Motor vehicle (road)",
                                              "Car",
                                              "Vehicle horn, car horn, honking",
                                              "Toot",
                                              "Car alarm",
                                              "Power windows, electric windows",
                                              "Skidding",
                                              "Tire squeal",
                                              "Car passing by",
                                              "Race car, auto racing",
                                              "Truck",
                                              "Air brake",
                                              "Air horn, truck horn",
                                              "Reversing beeps",
                                              "Ice cream truck, ice cream van",
                                              "Bus",
                                              "Emergency vehicle",
                                              "Police car (siren)",
                                              "Ambulance (siren)",
                                              "Fire engine, fire truck (siren)",
                                              "Motorcycle",
                                              "Traffic noise, roadway noise",
                                              "Rail transport",
                                              "Train",
                                              "Train whistle",
                                              "Train horn",
                                              "Railroad car, train wagon",
                                              "Train wheels squealing",
                                              "Subway, metro, underground",
                                              "Aircraft",
                                              "Aircraft engine",
                                              "Jet engine",
                                              "Propeller, airscrew",
                                              "Helicopter",
                                              "Fixed-wing aircraft, airplane",
                                              "Bicycle",
                                              "Skateboard",
                                              "Engine",
                                              "Light engine (high frequency)",
                                              "Dental drill, dentist's drill",
                                              "Lawn mower",
                                              "Chainsaw",
                                              "Medium engine (mid frequency)",
                                              "Heavy engine (low frequency)",
                                              "Engine knocking",
                                              "Engine starting",
                                              "Idling",
                                              "Accelerating, revving, vroom",
                                              "Door",
                                              "Doorbell",
                                              "Ding-dong",
                                              "Sliding door",
                                              "Slam",
                                              "Knock",
                                              "Tap",
                                              "Squeak",
                                              "Cupboard open or close",
                                              "Drawer open or close",
                                              "Dishes, pots, and pans",
                                              "Cutlery, silverware",
                                              "Chopping (food)",
                                              "Frying (food)",
                                              "Microwave oven",
                                              "Blender",
                                              "Water tap, faucet",
                                              "Sink (filling or washing)",
                                              "Bathtub (filling or washing)",
                                              "Hair dryer",
                                              "Toilet flush",
                                              "Toothbrush",
                                              "Electric toothbrush",
                                              "Vacuum cleaner",
                                              "Zipper (clothing)",
                                              "Keys jangling",
                                              "Coin (dropping)",
                                              "Scissors",
                                              "Electric shaver, electric razor",
                                              "Shuffling cards",
                                              "Typing",
                                              "Typewriter",
                                              "Computer keyboard",
                                              "Writing",
                                              "Alarm",
                                              "Telephone",
                                              "Telephone bell ringing",
                                              "Ringtone",
                                              "Telephone dialing, DTMF",
                                              "Dial tone",
                                              "Busy signal",
                                              "Alarm clock",
                                              "Siren",
                                              "Civil defense siren",
                                              "Buzzer",
                                              "Smoke detector, smoke alarm",
                                              "Fire alarm",
                                              "Foghorn",
                                              "Whistle",
                                              "Steam whistle",
                                              "Mechanisms",
                                              "Ratchet, pawl",
                                              "Clock",
                                              "Tick",
                                              "Tick-tock",
                                              "Gears",
                                              "Pulleys",
                                              "Sewing machine",
                                              "Mechanical fan",
                                              "Air conditioning",
                                              "Cash register",
                                              "Printer",
                                              "Camera",
                                              "Single-lens reflex camera",
                                              "Tools",
                                              "Hammer",
                                              "Jackhammer",
                                              "Sawing",
                                              "Filing (rasp)",
                                              "Sanding",
                                              "Power tool",
                                              "Drill",
                                              "Explosion",
                                              "Gunshot, gunfire",
                                              "Machine gun",
                                              "Fusillade",
                                              "Artillery fire",
                                              "Cap gun",
                                              "Fireworks",
                                              "Firecracker",
                                              "Burst, pop",
                                              "Eruption",
                                              "Boom",
                                              "Wood",
                                              "Chop",
                                              "Splinter",
                                              "Crack",
                                              "Glass",
                                              "Chink, clink",
                                              "Shatter",
                                              "Liquid",
                                              "Splash, splatter",
                                              "Slosh",
                                              "Squish",
                                              "Drip",
                                              "Pour",
                                              "Trickle, dribble",
                                              "Gush",
                                              "Fill (with liquid)",
                                              "Spray",
                                              "Pump (liquid)",
                                              "Stir",
                                              "Boiling",
                                              "Sonar",
                                              "Arrow",
                                              "Whoosh, swoosh, swish",
                                              "Thump, thud",
                                              "Thunk",
                                              "Electronic tuner",
                                              "Effects unit",
                                              "Chorus effect",
                                              "Basketball bounce",
                                              "Bang",
                                              "Slap, smack",
                                              "Whack, thwack",
                                              "Smash, crash",
                                              "Breaking",
                                              "Bouncing",
                                              "Whip",
                                              "Flap",
                                              "Scratch",
                                              "Scrape",
                                              "Rub",
                                              "Roll",
                                              "Crushing",
                                              "Crumpling, crinkling",
                                              "Tearing",
                                              "Beep, bleep",
                                              "Ping",
                                              "Ding",
                                              "Clang",
                                              "Squeal",
                                              "Creak",
                                              "Rustle",
                                              "Whir",
                                              "Clatter",
                                              "Sizzle",
                                              "Clicking",
                                              "Clickety-clack",
                                              "Rumble",
                                              "Plop",
                                              "Jingle, tinkle",
                                              "Hum",
                                              "Zing",
                                              "Boing",
                                              "Crunch",
                                              "Silence",
                                              "Sine wave",
                                              "Harmonic",
                                              "Chirp tone",
                                              "Sound effect",
                                              "Pulse",
                                              "Inside, small room",
                                              "Inside, large room or hall",
                                              "Inside, public space",
                                              "Outside, urban or manmade",
                                              "Outside, rural or natural",
                                              "Reverberation",
                                              "Echo",
                                              "Noise",
                                              "Environmental noise",
                                              "Static",
                                              "Mains hum",
                                              "Distortion",
                                              "Sidetone",
                                              "Cacophony",
                                              "White noise",
                                              "Pink noise",
                                              "Throbbing",
                                              "Vibration",
                                              "Television",
                                              "Radio",
                                              "Field recording"};
// 包含NCNN,FFTW头文件
#include "fftw3.h"
#include "mat.h"
#include "net.h"

// 定义YAMNet上下文结构
struct yamnet_context
{
    ncnn::Net net;
    bool initialized;
    float threshold;
};

// YAMNet 参数常量
namespace yamnet_params
{
    const int SAMPLE_RATE = 16000;
    const float STFT_WINDOW_SECONDS = 0.025f; // 25ms
    const float STFT_HOP_SECONDS = 0.010f;    // 10ms
    const int MEL_BANDS = 64;
    const float MEL_MIN_HZ = 125.0f;
    const float MEL_MAX_HZ = 7500.0f;
    const float LOG_OFFSET = 0.001f;
    const int PATCH_FRAMES = 96;
    const int PATCH_BANDS = 64;
} // namespace yamnet_params

// 预计算Mel滤波器缓存
static std::vector<std::vector<float>> mel_filterbank;
static bool mel_initialized = false;

// 简化的STFT实现
std::vector<std::vector<float>> compute_stft_magnitude(const std::vector<float> &audio, int n_fft,
                                                       int hop_length, int win_length)
{
    if (audio.empty())
    {
        return {};
    }

    // 实现center padding (与Python librosa对齐)
    int pad_amount = n_fft / 2;
    std::vector<float> padded_audio(audio.size() + 2 * pad_amount);

    // 反射填充开头
    for (int i = 0; i < pad_amount; ++i)
    {
        int src_idx = std::min(pad_amount - 1 - i, (int)audio.size() - 1);
        src_idx = std::max(0, src_idx);
        padded_audio[i] = audio[src_idx];
    }

    // 复制原始音频
    std::copy(audio.begin(), audio.end(), padded_audio.begin() + pad_amount);

    // 反射填充结尾
    for (int i = 0; i < pad_amount; ++i)
    {
        int src_idx = (int)audio.size() - 1 - (i % (int)audio.size());
        src_idx = std::max(0, std::min(src_idx, (int)audio.size() - 1));
        padded_audio[pad_amount + audio.size() + i] = audio[src_idx];
    }

    // 计算帧数
    int n_frames = (padded_audio.size() - win_length) / hop_length + 1;
    std::vector<std::vector<float>> magnitude(n_frames, std::vector<float>(n_fft / 2 + 1));

    // 优化的Hann窗 (提高精度)
    std::vector<float> window(win_length);
    for (int i = 0; i < win_length; ++i)
    {
        window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (win_length - 1)));
    }

    // 窗口归一化 (与librosa对齐)
    float window_sum = 0.0f;
    for (float w : window)
    {
        window_sum += w;
    }
    // 使用FFTW库进行FFT计算 (显著提高性能)
    float *in = (float *)fftwf_malloc(sizeof(float) * n_fft);
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (n_fft / 2 + 1));

    // 创建FFTW计划
    fftwf_plan p = fftwf_plan_dft_r2c_1d(n_fft, in, out, FFTW_ESTIMATE);

    for (int frame = 0; frame < n_frames; ++frame)
    {
        int start = frame * hop_length;

        // 清空输入数组
        std::memset(in, 0, sizeof(float) * n_fft);

        // 应用窗口函数
        for (int n = 0; n < win_length && start + n < padded_audio.size(); ++n)
        {
            in[n] = padded_audio[start + n] * window[n];
        }

        // 执行FFT
        fftwf_execute(p);

        // 计算幅度谱
        for (int k = 0; k < n_fft / 2 + 1; ++k)
        {
            float real = out[k][0];
            float imag = out[k][1];
            magnitude[frame][k] = sqrtf(real * real + imag * imag) + 1e-10f;
        }
    }

    // 清理FFTW资源
    fftwf_destroy_plan(p);
    fftwf_free(in);
    fftwf_free(out);

    return magnitude;
}

// 简化的mel滤波器组
std::vector<std::vector<float>> create_mel_filterbank(int n_fft, int n_mels, float sr, float fmin,
                                                      float fmax)
{
    std::vector<std::vector<float>> mel_fb(n_mels, std::vector<float>(n_fft / 2 + 1, 0.0f));

    // mel scale转换
    auto hz_to_mel = [](float hz)
    { return 2595.0f * log10f(1.0f + hz / 700.0f); };
    auto mel_to_hz = [](float mel)
    { return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f); };

    float mel_min = hz_to_mel(fmin);
    float mel_max = hz_to_mel(fmax);

    std::vector<float> mel_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; ++i)
    {
        mel_points[i] = mel_min + (mel_max - mel_min) * i / (n_mels + 1);
    }

    std::vector<float> hz_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; ++i)
    {
        hz_points[i] = mel_to_hz(mel_points[i]);
    }

    std::vector<int> bin_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; ++i)
    {
        bin_points[i] = static_cast<int>(hz_points[i] * n_fft / sr);
    }

    // 构建三角滤波器
    for (int m = 0; m < n_mels; ++m)
    {
        int left = bin_points[m];
        int center = bin_points[m + 1];
        int right = bin_points[m + 2];

        for (int k = left; k <= right && k < n_fft / 2 + 1; ++k)
        {
            if (k < center)
            {
                if (center > left)
                {
                    mel_fb[m][k] = static_cast<float>(k - left) / (center - left);
                }
            }
            else
            {
                if (right > center)
                {
                    mel_fb[m][k] = static_cast<float>(right - k) / (right - center);
                }
            }
        }
    }

    return mel_fb;
}

// 提取特征
static std::vector<std::vector<float>> extract_features(const std::vector<float> &audio)
{
    int window_length_samples =
        static_cast<int>(yamnet_params::SAMPLE_RATE * yamnet_params::STFT_WINDOW_SECONDS);
    int hop_length_samples =
        static_cast<int>(yamnet_params::SAMPLE_RATE * yamnet_params::STFT_HOP_SECONDS);

    // 计算FFT长度 (下一个2的幂)
    int fft_length = 1;
    while (fft_length < window_length_samples)
    {
        fft_length *= 2;
    }

    // 计算STFT幅度谱
    auto magnitude =
        compute_stft_magnitude(audio, fft_length, hop_length_samples, window_length_samples);
    if (magnitude.empty())
    {
        LOG_ERROR("STFT计算失败");
        return {};
    }

    // 创建mel滤波器组
    auto mel_fb =
        create_mel_filterbank(fft_length, yamnet_params::MEL_BANDS, yamnet_params::SAMPLE_RATE,
                              yamnet_params::MEL_MIN_HZ, yamnet_params::MEL_MAX_HZ);

    // 应用mel滤波器 (改进数值稳定性)
    std::vector<std::vector<float>> mel_spectrogram(magnitude.size(),
                                                    std::vector<float>(yamnet_params::MEL_BANDS));

    for (size_t frame = 0; frame < magnitude.size(); ++frame)
    {
        for (int mel = 0; mel < yamnet_params::MEL_BANDS; ++mel)
        {
            float sum = 0.0f;
            for (size_t bin = 0; bin < magnitude[frame].size(); ++bin)
            {
                sum += magnitude[frame][bin] * mel_fb[mel][bin];
            }
            // 改进对数变换，确保数值稳定性
            float mel_energy = std::max(sum, yamnet_params::LOG_OFFSET * 0.1f);
            mel_spectrogram[frame][mel] = logf(mel_energy + yamnet_params::LOG_OFFSET);
        }
    }

    return mel_spectrogram;
}

// 创建YAMNet上下文
yamnet_context_t *yamnet_create_context(float threshold)
{
    yamnet_context_t *ctx = new yamnet_context_t();
    if (ctx)
    {
        ctx->initialized = false;
    }
    ctx->threshold = threshold;
    LOG_DEBUG("YAMNet: ctx=%p, threshold=%f\n", ctx, threshold);
    return ctx;
}

// 初始化YAMNet模型
int yamnet_init_model(yamnet_context_t *ctx, const char *model_path)
{
    if (!ctx || !model_path)
    {
        return -1;
    }
    // 模型文件路径
    std::string param_file = std::string(model_path) + ".param";
    std::string bin_file = std::string(model_path) + ".bin";

    // 优化NCNN设置
    ctx->net.opt.use_vulkan_compute = false;
    ctx->net.opt.use_fp16_packed = false;
    ctx->net.opt.use_fp16_storage = false;
    ctx->net.opt.use_fp16_arithmetic = false;
    ctx->net.opt.use_int8_storage = false;
    ctx->net.opt.use_int8_arithmetic = false;

    // 加载模型
    int ret = ctx->net.load_param(param_file.c_str());
    if (ret != 0)
    {
        LOG_ERROR("Failed to load param file: %s\n", param_file.c_str());
        return -1;
    }

    ret = ctx->net.load_model(bin_file.c_str());
    if (ret != 0)
    {
        LOG_ERROR("Failed to load bin file: %s\n", bin_file.c_str());
        return -1;
    }

    ctx->initialized = true;
    return 0;
}
// 关键动物类别索引 (基于AudioSet标签)
const int animal_idx = 67;   // Animal
const int pets_idx = 68;     // Domestic animals, pets
const int canidae_idx = 117; // Canidae

const int dog_idx = 69;     // Dog
const int bark_idx = 70;    // Bark
const int yip_idx = 71;     // yip
const int howl_idx = 72;    // Howl
const int bow_wow_idx = 73; // Bow-wow
const int growl_idx = 74;   // Growl
const int whimper_idx = 75; // Whimper

const int cat_idx = 76;       // Cat
const int purr_idx = 77;      // Purr
const int meow_idx = 78;      // Meow
const int hiss_idx = 79;      // Hiss
const int caterwaul_idx = 80; // Caterwaul

const int speech_idx = 0;         // Speech
const int silence_idx = 494;      // Silence
const int inside_small_idx = 487; // Inside, small room

// #define YAMNET_DEBUG
// 动物声音后处理校准 (针对猫狗声优化)
static void apply_animal_sound_calibration(std::vector<float> &logits)
{
    if (logits.size() < 521)
        return;

    // 计算当前动物类别的平均激活
    float animal_activation = (logits[animal_idx] + logits[pets_idx]) / 2.0f;
    float dog_activation =
        (logits[dog_idx] + logits[bark_idx] + logits[yip_idx] + logits[howl_idx] +
         logits[bow_wow_idx] + logits[growl_idx] + logits[whimper_idx] + logits[canidae_idx]) /
        8.0f;
    float cat_activation = (logits[cat_idx] + logits[meow_idx] + logits[purr_idx] +
                            logits[hiss_idx] + logits[caterwaul_idx]) /
                           5.0f;
    float speech_activation = logits[speech_idx];
    float silence_activation = logits[silence_idx];
#ifdef DEBUG
    LOG_DEBUG("=========================\n");
    LOG_DEBUG("logits[animal_idx]: %f\n ", logits[animal_idx]);
    LOG_DEBUG("logits[pets_idx]: %f\n ", logits[pets_idx]);
    LOG_DEBUG("animal_activation: %f\n ", animal_activation);
    LOG_DEBUG("=========================\n");
    LOG_DEBUG("logits[dog_idx]: %f\n ", logits[dog_idx]);
    LOG_DEBUG("logits[bark_idx]: %f\n ", logits[bark_idx]);
    LOG_DEBUG("logits[yip_idx]: %f\n ", logits[yip_idx]);
    LOG_DEBUG("logits[howl_idx]: %f\n ", logits[howl_idx]);
    LOG_DEBUG("logits[bow_wow_idx]: %f\n ", logits[bow_wow_idx]);
    LOG_DEBUG("logits[growl_idx]: %f\n ", logits[growl_idx]);
    LOG_DEBUG("logits[whimper_idx]: %f\n ", logits[whimper_idx]);
    LOG_DEBUG("logits[canidae_idx]: %f\n ", logits[canidae_idx]);
    LOG_DEBUG("dog_activation: %f\n ", dog_activation);
    LOG_DEBUG("=========================\n");
    LOG_DEBUG("logits[cat_idx]: %f\n ", logits[cat_idx]);
    LOG_DEBUG("logits[meow_idx]: %f\n ", logits[meow_idx]);
    LOG_DEBUG("logits[purr_idx]: %f\n ", logits[purr_idx]);
    LOG_DEBUG("logits[hiss_idx]: %f\n ", logits[hiss_idx]);
    LOG_DEBUG("logits[caterwaul_idx]: %f\n ", logits[caterwaul_idx]);
    LOG_DEBUG("=========================\n");
#endif
    // 增强猫狗声音
    if (animal_activation > -1.0f || cat_activation > -2.0f ||
        dog_activation > -2.0f) // 动物声音阈值
    {
        // 增强猫相关类别 (如果有猫的特征)
        if (cat_activation > -2.0f)
        {
            logits[cat_idx] += 0.5f;       // 轻微增强Cat
            logits[meow_idx] += 0.3f;      // 轻微增强Meow
            logits[hiss_idx] += 0.3f;      // 轻微增强Hiss
            logits[purr_idx] += 0.3f;      // 轻微增强Purr
            logits[caterwaul_idx] += 0.3f; // 轻微增强Caterwaul
        }

        if (dog_activation > -2.0f)
        {
            logits[dog_idx] += 2.0f;     // 轻微增强Dog
            logits[bark_idx] += 1.0f;    // 轻微增强Bark
            logits[howl_idx] += 1.0f;    // 轻微增强Howl
            logits[yip_idx] += 1.0f;     // 轻微增强Yip
            logits[growl_idx] += 1.0f;   // 轻微增强Growl
            logits[whimper_idx] += 1.0f; // 轻微增强Whimper
            logits[canidae_idx] += 1.0f; // 轻微增强Canidae
        }
        logits[animal_idx] += 5.0f;
        logits[pets_idx] += 5.0f;
        // 降低非猫狗类别的概率
        for (size_t i = 0; i < logits.size(); i++)
        {
            if (i != cat_idx && i != meow_idx && i != hiss_idx && i != purr_idx &&
                i != caterwaul_idx && i != dog_idx && i != bark_idx && i != howl_idx &&
                i != yip_idx && i != growl_idx && i != whimper_idx && i != canidae_idx)
            {
                logits[i] -= 15.0f;
            }
        }
    }
    else
    {
        // 没有检测到宠物，降低猫狗类别的概率
        logits[cat_idx] -= 15.0f;
        logits[meow_idx] -= 15.0f;
        logits[hiss_idx] -= 15.0f;
        logits[purr_idx] -= 15.0f;
        logits[caterwaul_idx] -= 15.0f;
        logits[dog_idx] -= 15.0f;
        logits[bark_idx] -= 15.0f;
        logits[howl_idx] -= 15.0f;
        logits[yip_idx] -= 15.0f;
        logits[growl_idx] -= 15.0f;
        logits[whimper_idx] -= 15.0f;
        logits[canidae_idx] -= 15.0f;
    }
}

// 聚合预测结果：对每个种类按出现次数的平均概率
static std::vector<float>
aggregate_predictions(const std::vector<std::vector<std::pair<int, float>>> &all_topk,
                      size_t num_classes)
{
    if (all_topk.empty())
        return {};

    std::vector<float> aggregated(num_classes, 0.0f);
    std::vector<int> occurrence_count(num_classes, 0);

    // 仅累计各 patch 的 Top-K 项
    for (const auto &topk_pairs : all_topk)
    {
        for (const auto &kv : topk_pairs)
        {
            int cls = kv.first;
            float prob = kv.second;
            if (cls >= 0 && static_cast<size_t>(cls) < num_classes)
            {
                // aggregated[cls] += prob;
                aggregated[cls] = std::max(aggregated[cls], prob);
                occurrence_count[cls]++;
            }
        }
    }

    // // 用均值
    // for (size_t i = 0; i < aggregated.size(); ++i) {
    // 	if (occurrence_count[i] > 0) {
    // 		aggregated[i] /= occurrence_count[i];
    // 	} else {
    // 		aggregated[i] = 0.0f;
    // 	}
    // }

    return aggregated;
}

// 打印top-k结果
static int print_topk_audio(const std::vector<float> &logits, int topk)
{
    std::vector<std::pair<float, int>> prob_indices;

    // 直接使用已为概率的输入并排序
    for (size_t i = 0; i < logits.size() && i < class_names.size(); ++i)
    {
        float prob = std::max(0.0f, std::min(1.0f, logits[i]));
        prob_indices.push_back({prob, static_cast<int>(i)});
    }

    std::sort(prob_indices.begin(), prob_indices.end(),
              [](const std::pair<float, int> &a, const std::pair<float, int> &b)
              {
                  return a.first > b.first;
              });

    int actual_topk = std::min<int>(topk, prob_indices.size());

    return 0;
}

// 进行YAMNet推理（Top-K 全类别）
int yamnet_inference_topk(yamnet_context_t *ctx, const float *audio_data, int audio_length,
                          yamnet_detect_result_list_t *results)
{
    if (!ctx || !ctx->initialized || !audio_data || !results)
    {
        return -1;
    }

    memset(results, 0, sizeof(*results));

    yamnet_detect_result_list_t tmp_results;
    memset(&tmp_results, 0, sizeof(tmp_results));

    // 将音频数据转换为vector
    std::vector<float> audio_vec(audio_data, audio_data + audio_length);

    // 提取特征
    auto mel_spectrogram = extract_features(audio_vec);
    if (mel_spectrogram.empty())
    {
        return -1;
    }

    // 分割成patches (50%重叠)
    std::vector<std::vector<std::vector<float>>> patches;
    int num_frames = mel_spectrogram.size() - 1;
    LOG_DEBUG("num_frames: %d\n", num_frames);
    std::vector<std::vector<std::pair<int, float>>> all_topk;

    int patch_hop = yamnet_params::PATCH_FRAMES / 2; // 50%重叠
    for (int i = 0; i <= num_frames - yamnet_params::PATCH_FRAMES; i += patch_hop)
    {
        std::vector<std::vector<float>> patch(yamnet_params::PATCH_FRAMES,
                                              std::vector<float>(yamnet_params::PATCH_BANDS));

        for (int frame = 0; frame < yamnet_params::PATCH_FRAMES; ++frame)
        {
            if (i + frame < num_frames)
            {
                for (int band = 0; band < yamnet_params::PATCH_BANDS; ++band)
                {
                    patch[frame][band] = mel_spectrogram[i + frame][band];
                }
            }
        }
        patches.push_back(patch);
    }

    LOG_DEBUG("patches.size(): %d\n", patches.size());
    if (patches.empty())
    {
        return -1;
    }

    auto topk = 5;
    // 对每个patch进行推理
    for (size_t i = 0; i < patches.size(); ++i)
    {
        // 创建输入Mat (CHW格式: channels=1, height=96, width=64)
        ncnn::Mat input_mat(yamnet_params::PATCH_BANDS, yamnet_params::PATCH_FRAMES, 1);

        // 填充数据
        for (int h = 0; h < yamnet_params::PATCH_FRAMES; ++h)
        {
            for (int w = 0; w < yamnet_params::PATCH_BANDS; ++w)
            {
                input_mat.channel(0)[h * yamnet_params::PATCH_BANDS + w] = patches[i][h][w];
            }
        }

        // 创建推理器
        ncnn::Extractor ex = ctx->net.create_extractor();
        ex.set_light_mode(true);

        // 输入数据
        ex.input("in0", input_mat);

        // 提取输出
        ncnn::Mat output_mat;
        ex.extract("out0", output_mat);

        // 保存logits并应用后处理校准
        std::vector<float> logits(output_mat.w);
        for (int j = 0; j < output_mat.w; ++j)
        {
            logits[j] = output_mat[j];
        }

        // 特征后处理校准 (针对动物声音优化) 不需要优化
        // apply_animal_sound_calibration(logits);

        // 转为概率并保存
        std::vector<std::pair<float, int>> prob_indices;
        for (size_t k = 0; k < logits.size() && k < class_names.size(); ++k)
        {
            float prob = 1.0f / (1.0f + expf(-std::max(-500.0f, std::min(500.0f, logits[k]))));
            prob_indices.push_back({prob, static_cast<int>(k)});
        }

        std::sort(prob_indices.begin(), prob_indices.end(),
                  [](const std::pair<float, int> &a, const std::pair<float, int> &b)
                  {
                      return a.first > b.first;
                  });
        // 调试专用
        // #ifdef YAMNET_DEBUG
        // 		float patch_hop_seconds =
        // 		    (yamnet_params::PATCH_FRAMES / 2) * yamnet_params::STFT_HOP_SECONDS;
        // 		float time_start = i * patch_hop_seconds;
        // 		float time_end = time_start + yamnet_params::PATCH_FRAMES * yamnet_params::STFT_HOP_SECONDS;
        // 		LOG_DEBUG("📍 时间段 %d [%fs - %fs]:\n", (i + 1), (time_start - 0.48), (time_end - 0.48));
        // 		// 显示每个patch的top-k预测
        // 		LOG_DEBUG("=========================\n");
        // 		for (int j = 0; j < topk; ++j) {
        // 			std::string label = class_names[prob_indices[j].second];
        // 			LOG_DEBUG("  %d. %s %f%%\n", (j + 1), label.c_str(), prob_indices[j].first * 100);
        // 		}
        // 		LOG_DEBUG("=========================\n");
        // #endif

        if (patches.size() > 1)
        {
            // 保存本 patch Top-K (cls, prob)
            int actual_topk_patch = std::min<int>(topk, prob_indices.size());
            std::vector<std::pair<int, float>> topk_pairs;
            topk_pairs.reserve(actual_topk_patch);
            for (int j = 0; j < actual_topk_patch; ++j)
            {
                topk_pairs.push_back({prob_indices[j].second, prob_indices[j].first});
            }
            all_topk.push_back(topk_pairs);
        }
        else
        {
            for (int j = 0; j < topk; ++j)
            {
                tmp_results.results[j].cls_id = prob_indices[j].second;
                tmp_results.results[j].confidence = prob_indices[j].first;
                tmp_results.count++;
            }
        }
    }
    if (patches.size() > 1)
    {
        auto probs = aggregate_predictions(all_topk, class_names.size());
        print_topk_audio(probs, topk);
        std::vector<std::pair<float, int>> prob_indices;
        // 直接使用聚合后的概率进行排序（不再二次sigmoid）
        for (size_t i = 0; i < probs.size() && i < class_names.size(); ++i)
        {
            float prob = std::max(0.0f, std::min(1.0f, probs[i]));
            prob_indices.push_back({prob, static_cast<int>(i)});
        }

        std::sort(prob_indices.begin(), prob_indices.end(),
                  [](const std::pair<float, int> &a, const std::pair<float, int> &b)
                  {
                      return a.first > b.first;
                  });
        int actual_topk = std::min<int>(topk, prob_indices.size());
#ifdef YAMNET_DEBUG
        LOG_DEBUG("🎯 音频整体包含的声音类型 (Top-%d):\n", actual_topk);
        LOG_DEBUG("=========================\n");
#endif
        for (int i = 0; i < actual_topk; ++i)
        {
            tmp_results.results[i].cls_id = prob_indices[i].second;
            tmp_results.results[i].confidence = prob_indices[i].first;
            tmp_results.count++;
#ifdef YAMNET_DEBUG
            LOG_DEBUG("  %d. %s %f%%\n", (i + 1),
                      yamnet_get_class_name(tmp_results.results[i].cls_id),
                      tmp_results.results[i].confidence * 100);
#endif
        }
#ifdef YAMNET_DEBUG
        LOG_DEBUG("=========================\n");
#endif
    }
    for (int i = 0; i < tmp_results.count; i++)
    {
        int cls_id = tmp_results.results[i].cls_id;
        float confidence = tmp_results.results[i].confidence;

        results->results[results->count].cls_id = cls_id;
        results->results[results->count].confidence = confidence;
        results->count++;
    }
#ifdef DEBUG
    LOG_DEBUG("=======================================================\n");
    for (size_t i = 0; i < results->count; i++)
    {
        LOG_INFO("YAMNet results detected %d sounds %s %f\n", results->results[i].cls_id,
                 yamnet_get_class_name(results->results[i].cls_id), results->results[i].confidence);
    }
    LOG_DEBUG("=======================================================\n");
#endif

    return 0;
}

int yamnet_inference(yamnet_context_t *ctx, const float *audio_data, int audio_length,
                     yamnet_detect_result_list_t *results)
{
    yamnet_detect_result_list_t all_results;
    memset(&all_results, 0, sizeof(all_results));

    int ret = yamnet_inference_topk(ctx, audio_data, audio_length, &all_results);
    if (ret != 0 || !results)
    {
        return ret;
    }

    memset(results, 0, sizeof(*results));
    for (int i = 0; i < all_results.count; i++)
    {
        int cls_id = all_results.results[i].cls_id;
        float confidence = all_results.results[i].confidence;
        bool cat_dog = false;

        if (cls_id == dog_idx || cls_id == bark_idx || cls_id == yip_idx || cls_id == howl_idx ||
            cls_id == bow_wow_idx || cls_id == growl_idx || cls_id == whimper_idx ||
            cls_id == cat_idx || cls_id == purr_idx || cls_id == meow_idx || cls_id == hiss_idx ||
            cls_id == caterwaul_idx || cls_id == canidae_idx)
        {
            cat_dog = true;
        }

        if (cat_dog)
        {
            results->results[results->count].cls_id = cls_id;
            results->results[results->count].confidence = confidence;
            results->count++;
        }
    }
#ifdef DEBUG
    LOG_DEBUG("=======================================================\n");
    for (size_t i = 0; i < results->count; i++)
    {
        LOG_INFO("YAMNet results detected %d sounds %s %f\n", results->results[i].cls_id,
                 yamnet_get_class_name(results->results[i].cls_id), results->results[i].confidence);
    }
    LOG_DEBUG("=======================================================\n");
#endif

    return 0;
}

int yamnet_check_cat_dog(yamnet_context_t *ctx, yamnet_detect_result_list_t *results)
{
    int detected = -1;
    float cat_confidence = 0.0f;
    float dog_confidence = 0.0f;
    for (int i = 0; i < results->count; i++)
    {
        if ((results->results[i].cls_id == dog_idx || results->results[i].cls_id == bark_idx ||
             results->results[i].cls_id == yip_idx || results->results[i].cls_id == howl_idx ||
             results->results[i].cls_id == bow_wow_idx || results->results[i].cls_id == growl_idx ||
             results->results[i].cls_id == whimper_idx ||
             results->results[i].cls_id == canidae_idx) &&
            results->results[i].confidence > ctx->threshold)
        {
            memcpy(results->results[i].cls_name, "dog", sizeof(results->results[i].cls_name));
            LOG_DEBUG("检测到狗叫声，置信度: %.2f , 类别名称: %s, 阈值: %.2f\n",
                      results->results[i].confidence,
                      yamnet_get_class_name(results->results[i].cls_id), ctx->threshold);
            dog_confidence += results->results[i].confidence;
        }
        else if ((results->results[i].cls_id == cat_idx ||
                  results->results[i].cls_id == meow_idx ||
                  results->results[i].cls_id == purr_idx ||
                  results->results[i].cls_id == hiss_idx ||
                  results->results[i].cls_id == caterwaul_idx) &&
                 results->results[i].confidence > ctx->threshold)
        {
            // 目前不需要猫叫了，只处理狗叫
            // memcpy(results->results[i].cls_name, "cat", sizeof(results->results[i].cls_name));
            // LOG_DEBUG("检测到猫叫声，置信度: %.2f, 类别名称: %s, 阈值: %.2f\n",
            //           results->results[i].confidence,
            //           yamnet_get_class_name(results->results[i].cls_id), ctx->threshold);
            // cat_confidence += results->results[i].confidence;
        }
    }
    if (dog_confidence > cat_confidence)
    {
        detected = 0;
    }
    else if (cat_confidence > dog_confidence)
    {
        // detected = 1;
    }
    if (detected != -1)
    {
        LOG_DEBUG("检测到动物叫声: %d, 总猫叫声置信度: %.2f, 总狗叫声置信度: %.2f\n", detected,
                  cat_confidence, dog_confidence);
    }
    return detected;
}

// 销毁YAMNet上下文
void yamnet_destroy_context(yamnet_context_t *ctx)
{
    if (ctx)
    {
        delete ctx;
    }
}

// 获取类别名称
const char *yamnet_get_class_name(int cls_id)
{
    if (cls_id >= 0 && cls_id < (int)class_names.size())
    {
        return class_names[cls_id].c_str();
    }
    return "unknown";
}

// 初始化后处理
int yamnet_init_post_process() { return 0; }

// 清理后处理
void yamnet_deinit_post_process(void) {}

// 音频预处理函数
int yamnet_preprocess_audio(const int16_t *pcm_data, int pcm_length, float *audio_data)
{
    if (!pcm_data || !audio_data || pcm_length <= 0)
    {
        return -1;
    }

    // 将int16_t PCM数据转换为浮点数据，并进行归一化
    // YAMNet期望输入在[-1, 1]范围内
    const float scale = 1.0f / 32768.0f;

    for (int i = 0; i < pcm_length; i++)
    {
        audio_data[i] = pcm_data[i] * scale;
    }

    return 0;
}