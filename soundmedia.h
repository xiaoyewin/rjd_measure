#ifndef SOUNDMEDIA_H
#define SOUNDMEDIA_H


//声音播放的头文件
#include <QMediaPlayer>
#include <QSoundEffect>
typedef enum _sound_type
{
    START_SOUND = 0 ,
    STOP_SOUND ,
    SAMPLE_SOUND,
    POWERUP_SOUND

}T_Sound_type;



class SoundMedia
{
public:
    SoundMedia();
    ~SoundMedia();
    void set_play_sound(T_Sound_type sound_type);  //设置并且播放声音

private:
    //声音这块
    QMediaPlayer *Sound_Player;
    QSoundEffect *Sound_effect;
    void set_sound_volume(short sound_value);
};

#endif // SOUNDMEDIA_H
