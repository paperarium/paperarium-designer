#ifndef GPU_COMMON_H
#define GPU_COMMON_H

// PLATFORM-SPECIFIC window handle information
// Use this awful solution until I integrate GHOST in the application.
//  - PLATFORM_WIN_PARAMS: used in place of parameters
//  - PLATFORM_WIN_VARS: used in place of member variables
//  - PLATFORM_WIN_ARGS: used to funnel through params as args
//  - PLATFORM_WIN_ARGS: used to funnel through member vars as args

#if defined(VK_USE_PLATFORM_WIN32_KHR)  // windows

#define PLATF_SURF_PARAMS HINSTANCE platformHandle, HWND platformWindow
#define PLATF_SURF_MEMBERS     \
  HINSTANCE m_platform_handle; \
  HWND m_platform_window;
#define PLATF_SURF_ARGS platformHandle, platformWindow
#define PLATF_SURF_MEMB_ARGS m_platform_handle, m_platform_window
#define PLATF_SURF_INITIALIZERS \
  m_platform_handle(platformHandle), m_platform_window(platformWindow)

#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)  // directfb

#define PLATF_SURF_PARAMS \
  IDirectFB *platformDfb, IDirectFBSurface *platformDfbSurface
#define PLATF_SURF_MEMBERS   \
  IDirectFB* m_platform_dfb; \
  IDirectFBSurface* m_platform_dfb_surface;
#define PLATF_SURF_ARGS platformDfb, platformDfbSurface
#define PLATF_SURF_MEMB_ARGS m_platform_dfb, m_platform_dfb_surface
#define PLATF_SURF_INITIALIZERS \
  m_platform_dfb(platformDfb), m_platform_dfb_surface(platformDfbSurface)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)  // wayland

#define PLATF_SURF_PARAMS \
  wl_display *platformDisplay, wl_surface *platformSurface
#define PLATF_SURF_MEMBERS        \
  wl_display* m_platform_display; \
  wl_surface* m_platform_surface;
#define PLATF_SURF_ARGS platformDisplay, platformSurface
#define PLATF_SURF_MEMB_ARGS m_platform_display, m_platform_surface
#define PLATF_SURF_INITIALIZERS \
  m_platform_display(platformDisplay), m_platform_surface(platformSurface)

#elif defined(VK_USE_PLATFORM_XCB_KHR)  // XCB

#define PLATF_SURF_PARAMS \
  xcb_connection_t *platformConnection, xcb_window_t *platformWindow
#define PLATF_SURF_MEMBERS                 \
  xcb_connection_t* m_platform_connection; \
  xcb_window_t* m_platform_window;
#define PLATF_SURF_ARGS platformConnection, platformWindow
#define PLATF_SURF_MEMB_ARGS m_platform_connection, m_platform_window
#define PLATF_SURF_INITIALIZERS \
  m_platform_connection(platformConnection), m_platform_window(platformWindow)

#elif defined(VK_USE_PLATFORM_IOS_MVK) || \
    defined(VK_USE_PLATFORM_MACOS_MVK)  // macOS / iOS

#define PLATF_SURF_PARAMS void* platformView
#define PLATF_SURF_MEMBERS void* m_platform_view;
#define PLATF_SURF_ARGS platformView
#define PLATF_SURF_MEMB_ARGS m_platform_view
#define PLATF_SURF_INITIALIZERS m_platform_view(platformView)

#elif defined(_DIRECT2DISPLAY) || \
    defined(VK_USE_PLATFORM_HEADLESS_EXT)  // D2D / headless

#define PLATF_SURF_PARAMS uint32_t platformWidth, uint32_t platformHeight
#define PLATF_SURF_MEMBERS   \
  uint32_t m_platform_width; \
  uint32_t m_platform_height;
#define PLATF_SURF_ARGS platformWidth, platformHeight
#define PLATF_SURF_MEMB_ARGS m_platform_width, m_platform_height
#define PLATF_SURF_INITIALIZERS \
  m_platform_width(platformWidth), m_platform_height(platformHeight)

#endif

#endif /* GPU_COMMON_H */
