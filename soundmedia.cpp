#include "soundmedia.h"

SoundMedia::SoundMedia()
{
    //播放声音
    Sound_Player=new QMediaPlayer;
    //Sound_effect=new QSoundEffect;

    set_sound_volume(100);

}

void SoundMedia::set_sound_volume(short sound_value)
{
    Sound_Player->setVolume(sound_value);
   // Sound_effect->setVolume((float)sound_value/100); //音量  0~1之间
}

void SoundMedia::set_play_sound(T_Sound_type sound_type)
{
    switch(sound_type)
    {


      case START_SOUND:
            Sound_Player->setMedia(QUrl("qrc:/sound/start.wav"));
           // Sound_effect->setSource(QUrl("qrc:/sound/sample.wav"));
            break;
       case STOP_SOUND:
            Sound_Player->setMedia(QUrl("qrc:/sound/stop.wav"));
            //Sound_effect->setSource(QUrl("qrc:/sound/sample.wav"));
            break;
       case SAMPLE_SOUND:
            Sound_Player->setMedia(QUrl("qrc:/sound/sample.wav"));
           // Sound_effect->setSource(QUrl("qrc:/sound/sample.wav"));
             break;
       case POWERUP_SOUND:
             Sound_Player->setMedia(QUrl("qrc:/sound/sample.wav"));
            // Sound_effect->setSource(QUrl("qrc:/sound/sample.wav"));
             break;
    default:
         break;
    }

   // Sound_effect->setLoopCount(1);  //循环次数
    //Sound_effect->play();

    Sound_Player->play();
}

SoundMedia::~SoundMedia()
{
    delete Sound_Player;
   // delete Sound_effect;
}
