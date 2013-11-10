#include <Windows.h>
#include "WindowViewer.h"
#include <unordered_map>
#include "renderer/Camera.h"

using namespace std;

namespace {
  wstring widen(const std::string &src) {
	  wchar_t *wcs = new wchar_t[src.length() + 1];
	  mbstowcs(wcs, src.c_str(), src.length() + 1);
	  wstring dest = wcs;
	  delete [] wcs;
    return dest;
  }
}

namespace OmochiRenderer {
  class WindowViewer::WindowImpl {
    WindowViewer &viewer;
    static unordered_map<size_t, WindowImpl *> handleToInstance;

  public:
    explicit WindowImpl(WindowViewer &viewer_)
      : viewer(viewer_)
    {
    }

    bool CreateNewWindow() {
      HWND hWnd = CreateNewWindow_impl();
      if (hWnd != INVALID_HANDLE_VALUE) {
        RegisterWindow(hWnd, this);
        return true;
      }
      return false;
    }

    static void MessageLoop() {
      // ���b�Z�[�W���[�v
      MSG msg;
	    while(true)
	    {
		    BOOL ret = GetMessage( &msg, NULL, 0, 0 );  // ���b�Z�[�W���擾����
		    if( ret == 0 || ret == -1 )
		    {
			    // �A�v���P�[�V�������I�������郁�b�Z�[�W�����Ă�����A
			    // ���邢�� GetMessage() �����s������( -1 ���Ԃ��ꂽ�� �j�A���[�v�𔲂���
			    break;
		    }
		    else
		    {
			    // ���b�Z�[�W����������
			    TranslateMessage( &msg );
			    DispatchMessage( &msg );
		    }
	    }

    }

  private:
    HWND CreateNewWindow_impl() {
      WNDCLASSEX wc;
      HWND hWnd;
      HINSTANCE hInst = static_cast<HINSTANCE>(GetModuleHandle(NULL));

      // �E�B���h�E�N���X�̏���ݒ�
      wc.cbSize = sizeof(wc);               // �\���̃T�C�Y
      wc.style = CS_HREDRAW | CS_VREDRAW;   // �X�^�C��
      wc.lpfnWndProc = WindowImpl::WndProc;             // �E�B���h�E�v���V�[�W��
      wc.cbClsExtra = 0;                    // �g�����P
      wc.cbWndExtra = 0;                    // �g�����Q
      wc.hInstance = hInst;                 // �C���X�^���X�n���h��
      wc.hIcon = (HICON)LoadImage(          // �A�C�R��
	      NULL, MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON,
	      0, 0, LR_DEFAULTSIZE | LR_SHARED
      );
      wc.hIconSm = wc.hIcon;                // �q�A�C�R��
      wc.hCursor = (HCURSOR)LoadImage(      // �}�E�X�J�[�\��
	      NULL, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR,
	      0, 0, LR_DEFAULTSIZE | LR_SHARED
      );
      wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // �E�B���h�E�w�i
      wc.lpszMenuName = NULL;                     // ���j���[��
      // ugly code
      wc.lpszClassName = widen(viewer.m_windowTitle.c_str()).c_str();// �E�B���h�E�N���X��

      // �E�B���h�E�N���X��o�^����
      if( RegisterClassEx( &wc ) == 0 ){ return (HWND)INVALID_HANDLE_VALUE; }

      int width = viewer.m_camera.GetScreenWidth();
      int height = viewer.m_camera.GetScreenHeight();

      int x = ( GetSystemMetrics( SM_CXSCREEN ) - width ) / 2;
      int y = ( GetSystemMetrics( SM_CYSCREEN ) - height ) / 2;

      // �E�B���h�E���쐬����
      hWnd = CreateWindow(
	      wc.lpszClassName,      // �E�B���h�E�N���X��
        widen(viewer.m_windowTitle.c_str()).c_str(),  // �^�C�g���o�[�ɕ\�����镶����
	      WS_OVERLAPPEDWINDOW,   // �E�B���h�E�̎��
	      x,         // �E�B���h�E��\������ʒu�iX���W�j
	      y,         // �E�B���h�E��\������ʒu�iY���W�j
	      width,         // �E�B���h�E�̕�
	      height,         // �E�B���h�E�̍���
	      NULL,                  // �e�E�B���h�E�̃E�B���h�E�n���h��
	      NULL,                  // ���j���[�n���h��
	      hInst,                 // �C���X�^���X�n���h��
	      NULL                   // ���̑��̍쐬�f�[�^
      );


      if (hWnd == INVALID_HANDLE_VALUE) return (HWND)INVALID_HANDLE_VALUE;

      RECT wrect, crect;
      GetWindowRect(hWnd, &wrect); GetClientRect(hWnd, &crect);
      int truewidth = width + (wrect.right-wrect.left) - (crect.right-crect.left);
      int trueheight = height + (wrect.bottom-wrect.top) - (crect.bottom-crect.top);
      x = (GetSystemMetrics(SM_CXSCREEN) - truewidth)/2;
      y = (GetSystemMetrics(SM_CYSCREEN) - trueheight)/2;
      MoveWindow(hWnd, x, y, truewidth, trueheight, TRUE);

      // �E�B���h�E��\������
	    ShowWindow( hWnd, SW_SHOW );
	    UpdateWindow( hWnd );

      return hWnd;
    }

    static void RegisterWindow(HWND hWnd, WindowImpl *instance) {
      size_t sizethWnd = reinterpret_cast<size_t>(hWnd);
      handleToInstance[sizethWnd] = instance;
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
      size_t sizethWnd = reinterpret_cast<size_t>(hWnd);
      if (handleToInstance.find(sizethWnd) == handleToInstance.end()) {
        return DefWindowProc( hWnd, msg, wp, lp );
      }

      return handleToInstance[sizethWnd]->WndProc_impl(hWnd, msg, wp, lp);
    }

  private:
    LRESULT WndProc_impl(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
      switch (msg) {
      case WM_DESTROY:
		      PostQuitMessage( 0 );
		      return 0;
      }

      return DefWindowProc( hWnd, msg, wp, lp );
    }
  };

  unordered_map<size_t, WindowViewer::WindowImpl *> WindowViewer::WindowImpl::handleToInstance;

  WindowViewer::WindowViewer(const std::string &windowTitle, const Camera &camera, const PathTracer &renderer)
    : m_windowTitle(windowTitle)
    , m_camera(camera)
    , m_renderer(renderer)
    , m_pWindow(NULL)
  {
  }
  WindowViewer::~WindowViewer() {
  }

  void WindowViewer::StartViewerOnThisThread() {
    m_pWindow.reset(new WindowImpl(*this));
    m_pWindow->CreateNewWindow();

    WindowImpl::MessageLoop();
  }

  void WindowViewer::StartViewerOnNewThread() {
  }

  bool WindowViewer::CreateViewWindow() {
  }
}
