#pragma once

#include "Image.h"

namespace OmochiRenderer
{
  // �摜�̓��o�́A�������Ǘ�����N���X
  class ImageHandler
  {
  public:
    // �摜�ɑ΂��ă��j�[�N�ɕt�^�����ID
    typedef int IMAGE_ID;
    // �L���łȂ��摜ID
    static const IMAGE_ID INVALID_IMAGE_ID = -1;
  private:
    ImageHandler() : m_filenameToImageIndex(), m_images() {}
    ~ImageHandler();

    std::unordered_map<std::string, IMAGE_ID> m_filenameToImageIndex;
    std::vector<Image *> m_images;
  public:
    static ImageHandler & GetInstance() {
      static ImageHandler s;
      return s;
    }

    // �O���t�@�C������̓ǂݍ��݁Bpng, jpg, bmp���ɑΉ� (hdr�͂����ł͔�Ή�)
    IMAGE_ID LoadFromFile(const std::string &fname, bool doReverseGamma = true);
    // ��̉摜�쐬
    IMAGE_ID CreateImage(size_t width, size_t height);
    // �w�肵��ID�ɕR�Â������̎擾
    Image * GetImage(IMAGE_ID id);
    const Image * GetImage(IMAGE_ID id) const;
    // �摜���
    void ReleaseImage(IMAGE_ID id);
    // PNG �t�@�C���ւ̕ۑ�
    bool SaveToPngFile(const std::string &fname, const Image *image) const;
    // PPM �t�@�C���ւ̕ۑ�
    bool SaveToPpmFile(const std::string &fname, const Image *image) const;
  };
}
