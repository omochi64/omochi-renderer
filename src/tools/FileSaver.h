#pragma once

#include <memory>

#include "renderer/Settings.h"

namespace OmochiRenderer {

  // �t�@�C���ۑ��̂��߂̊��N���X�B���̉��s��
  class FileSaver {
  public:
    explicit FileSaver(std::shared_ptr<Settings> settings)
      : m_settings(settings)
    {
    }

    // �ۑ����ɌĂ΂��֐�
    virtual void Save(int samples, const Color *img, double accumulatedPastTime) const = 0;

  protected:

    // settings ��ۑ����̌Ăяo����������t�@�C�����𐶐���������p���\�b�h
    std::string P_CreateFileName(int samples, double accumulatedPastTime) const {
      char name[1024];
      const static std::string file_format_key = "save filename format for pathtracer";
      if (m_settings->GetRawSetting(file_format_key).empty()) {
        sprintf_s(name, 1024, "result_ibl_test_w%d_h%d_%04d_%dx%d_%.2fmin.ppm",
          m_settings->GetWidth(), m_settings->GetHeight(), samples, m_settings->GetSuperSamples(), m_settings->GetSuperSamples(), accumulatedPastTime);
      } else {
        sprintf_s(name, 1024, m_settings->GetRawSetting(file_format_key).c_str(),
          m_settings->GetWidth(), m_settings->GetHeight(), samples, m_settings->GetSuperSamples(), m_settings->GetSuperSamples(), accumulatedPastTime);
      }

      return name;
    }

    std::shared_ptr<Settings> m_settings;
  };

}

