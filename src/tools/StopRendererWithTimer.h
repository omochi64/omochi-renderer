#pragma once

#include <thread>

namespace OmochiRenderer {
  class Renderer;

  class StopRendererWithTimer {
  public:
    explicit StopRendererWithTimer(std::weak_ptr<Renderer> renderer);
    ~StopRendererWithTimer();

    void SetTimer(double timeToStop) {
      m_timeToStop = timeToStop;
    }

    // �ʃX���b�h�ł̃^�C�}�[�Ď����J�n����B�K�v�Ȑݒ肪�s���Ă��Ȃ������ꍇ�͎��s���� false ���Ԃ��Ă���
    bool StartTimer();

  private:
    std::weak_ptr<Renderer> m_renderer;

    std::shared_ptr<std::thread> m_thread;

    double m_timeToStop;
  };
}