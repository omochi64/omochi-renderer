#pragma once
#include <thread>

namespace OmochiRenderer {
  
  class FileSaver;
  class Renderer;
  
  class FileSaverCallerWithTimer {
  public:
    explicit FileSaverCallerWithTimer(std::weak_ptr<Renderer> renderer, std::shared_ptr<FileSaver> saver);

    void SetSaveTimerInformation(double saveSpanInSec) {
      m_saveSpan = saveSpanInSec;
    }

    void AddSaver(const std::shared_ptr<FileSaver> &saver) {
      m_savers.push_back(saver);
    }

    // �ʃX���b�h�ł̃^�C�}�[�Ď����J�n����B�K�v�Ȑݒ肪�s���Ă��Ȃ������ꍇ�͎��s���� false ���Ԃ��Ă���
    bool StartTimer();

    // �^�C�}�[�X�g�b�v�𖽗߂��A�X�g�b�v����܂ő҂�
    void StopAndWaitStopping();

  private:
    std::weak_ptr<Renderer> m_renderer;
    std::vector<std::shared_ptr<FileSaver>> m_savers;

    std::shared_ptr<std::thread> m_thread;

    bool m_stopSignal;
    double m_saveSpan;
    double m_lastSaveTime;
  };
}
