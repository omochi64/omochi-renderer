#include "stdafx.h"

#include <Windows.h>

#include "StopRendererWithTimer.h"
#include "renderer/Renderer.h"

namespace OmochiRenderer {

  StopRendererWithTimer::StopRendererWithTimer(std::weak_ptr<Renderer> renderer)
    : m_renderer(renderer)
    , m_thread()
    , m_timeToStop(0)
  {
  }

  StopRendererWithTimer::~StopRendererWithTimer() {
    if (m_thread && m_thread->joinable()) {
      m_thread->join();
    }
    m_thread.reset();
  }

  // �ʃX���b�h�ł̃^�C�}�[�Ď����J�n����B�K�v�Ȑݒ肪�s���Ă��Ȃ������ꍇ�͎��s���� false ���Ԃ��Ă���
  bool StopRendererWithTimer::StartTimer() {

    if (m_timeToStop <= 0) return false;

    m_thread = std::make_shared<std::thread>(
      [this]() {

        DWORD timeInMsec = static_cast<DWORD>(m_timeToStop * 1000 + 0.9999);

        Sleep(timeInMsec);

        std::cerr << "Stop renderer by timer (" << m_timeToStop << " sec.)" << std::endl;

        if (auto renderer = m_renderer.lock()) {
          renderer->StopRendering();
        }

    });

    return true;
  }

}