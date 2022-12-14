#include <windows.h>        // Header File For Windows
#include <gl\glu.h>            // Header File For The GLu32 Library
#include "console.h"
#include "xmanimation.h"
#include "timer.h"

HDC hDC = NULL;        // Private GDI Device Context
HGLRC hRC = NULL;        // Permanent Rendering Context
HWND hWnd = NULL;        // Holds Our Window Handle
HINSTANCE hInstance;        // Holds The Instance Of The Application
ConsoleWindow con;
xmAnimation model;
xmTimer timer;

bool keys[256];            // Array Used For The Keyboard Routine
bool active = TRUE;        // Window Active Flag Set To TRUE By Default

LRESULT    CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);    // Declaration For WndProc
GLvoid ReSizeGLScene(GLsizei width, GLsizei height);        // Resize And Initialize The GL Window

int InitGL(GLvoid)                                        // All Setup For OpenGL Goes Here
{
    glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);                // Black Background
    glClearDepth(1.0f);                                    // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                            // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                                // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    OPENFILENAME ofn;
    char szFile[1024];
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    con.Open();
    if (GetOpenFileName(&ofn) == TRUE) {
        printf("%s\n", szFile);
        if (!model.xmLoadBinaryXMAnimation(szFile)) return false;
    } else return false;


    glEnable(GL_TEXTURE_2D);
    return TRUE;                                        // Initialization Went OK
}

float yrot = 0;
float xrot = 0;
float trans = -1;

int currAnim = 0;
bool bAnim = false;
vec2 angle = vec2(0.0, 0.0);

int DrawGLScene(GLvoid)                                    // Here's Where We Do All The Drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear Screen And Depth Buffer
    glLoadIdentity();                                    // Reset The Current Modelview Matrix
    timer.xmCalculateFrameRate();
    if (keys[VK_LEFT]) { yrot += 0.5; }
    if (keys[VK_RIGHT]) { yrot -= 0.5; }
    if (keys[VK_DOWN]) { xrot -= 0.5; }
    if (keys[VK_UP]) { xrot += 0.5; }
    if (keys[VK_SPACE]) { trans--; }
    glTranslatef(0, 0, trans);
    glRotatef(-yrot + 180, 0, 1, 0);
    glRotatef(xrot + 180, 1, 0, 0);

    if (GetAsyncKeyState('V')) angle.x += timer.fFrameInterval;
    if (GetAsyncKeyState('B')) angle.x -= timer.fFrameInterval;
    if (GetAsyncKeyState('N')) angle.y += timer.fFrameInterval;
    if (GetAsyncKeyState('M')) angle.y -= timer.fFrameInterval;

    if (angle.x > M_PI / 1.5 || angle.x < -M_PI / 1.5) angle.x = 0;
    if (angle.y > M_PI / 1.5 || angle.y < -M_PI / 1.5) angle.y = 0;

    if (GetAsyncKeyState('S') && !bAnim) {
        bAnim = true;
        if (currAnim + 1 >= model.iNumAnimations)
            currAnim = 0;
        else
            currAnim++;
        printf("%d\n", currAnim);
        model.fAnimationTime = model.anim[currAnim].animStartFrame;
    }
    if (!GetAsyncKeyState('S')) bAnim = false;
    model.setAnimationByID(currAnim, true);
    //model.setBlendedAnimation(0, 1, 50, true);
    model.Rotate(vec3(angle.x, 0.0, angle.y));
    model.draw();


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 0, 480, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_POINTS);
    {
        glVertex3f(320, 240, 0);
    }
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();


    return TRUE;                                        // Everything Went OK
}

GLvoid KillGLWindow(GLvoid)                                // Properly Kill The Window
{
    if (hRC)                                            // Do We Have A Rendering Context?
    {
        if (!wglMakeCurrent(NULL, NULL))                    // Are We Able To Release The DC And RC Contexts?
        {
            MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC))                        // Are We Able To Delete The RC?
        {
            MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;                                        // Set RC To NULL
    }

    if (hDC && !ReleaseDC(hWnd, hDC))                    // Are We Able To Release The DC
    {
        MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hDC = NULL;                                        // Set DC To NULL
    }

    if (hWnd && !DestroyWindow(hWnd))                    // Are We Able To Destroy The Window?
    {
        MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;                                        // Set hWnd To NULL
    }

    if (!UnregisterClass("OpenGL", hInstance))            // Are We Able To Unregister Class
    {
        MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;                                    // Set hInstance To NULL
    }
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char *title, int width, int height, int bits, bool fullscreenflag) {
    GLuint PixelFormat;            // Holds The Results After Searching For A Match
    WNDCLASS wc;                        // Windows Class Structure
    DWORD dwExStyle;                // Window Extended Style
    DWORD dwStyle;                // Window Style
    RECT WindowRect;                // Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left = (long) 0;            // Set Left Value To 0
    WindowRect.right = (long) width;        // Set Right Value To Requested Width
    WindowRect.top = (long) 0;                // Set Top Value To 0
    WindowRect.bottom = (long) height;        // Set Bottom Value To Requested Height

    hInstance = GetModuleHandle(NULL);                // Grab An Instance For Our Window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;    // Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc = (WNDPROC) WndProc;                    // WndProc Handles Messages
    wc.cbClsExtra = 0;                                    // No Extra Window Data
    wc.cbWndExtra = 0;                                    // No Extra Window Data
    wc.hInstance = hInstance;                            // Set The Instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);            // Load The Default Icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);            // Load The Arrow Pointer
    wc.hbrBackground = NULL;                                    // No Background Required For GL
    wc.lpszMenuName = NULL;                                    // We Don't Want A Menu
    wc.lpszClassName = "OpenGL";                                // Set The Class Name

    if (!RegisterClass(&wc))                                    // Attempt To Register The Window Class
    {
        MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                            // Return FALSE
    }


    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;            // Window Extended Style
    dwStyle = WS_OVERLAPPEDWINDOW;                            // Windows Style

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);        // Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd = CreateWindowEx(dwExStyle,                            // Extended Style For The Window
                                "OpenGL",                            // Class Name
                                title,                                // Window Title
                                dwStyle |                            // Defined Window Style
                                WS_CLIPSIBLINGS |                    // Required Window Style
                                WS_CLIPCHILDREN,                    // Required Window Style
                                0, 0,                                // Window Position
                                WindowRect.right - WindowRect.left,    // Calculate Window Width
                                WindowRect.bottom - WindowRect.top,    // Calculate Window Height
                                NULL,                                // No Parent Window
                                NULL,                                // No Menu
                                hInstance,                            // Instance
                                NULL)))                                // Dont Pass Anything To WM_CREATE
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    static PIXELFORMATDESCRIPTOR pfd =                // pfd Tells Windows How We Want Things To Be
            {
                    sizeof(PIXELFORMATDESCRIPTOR),                // Size Of This Pixel Format Descriptor
                    1,                                            // Version Number
                    PFD_DRAW_TO_WINDOW |                        // Format Must Support Window
                    PFD_SUPPORT_OPENGL |                        // Format Must Support OpenGL
                    PFD_DOUBLEBUFFER,                            // Must Support Double Buffering
                    PFD_TYPE_RGBA,                                // Request An RGBA Format
                    bits,                                        // Select Our Color Depth
                    0, 0, 0, 0, 0, 0,                            // Color Bits Ignored
                    0,                                            // No Alpha Buffer
                    0,                                            // Shift Bit Ignored
                    0,                                            // No Accumulation Buffer
                    0, 0, 0, 0,                                    // Accumulation Bits Ignored
                    16,                                            // 16Bit Z-Buffer (Depth Buffer)
                    0,                                            // No Stencil Buffer
                    0,                                            // No Auxiliary Buffer
                    PFD_MAIN_PLANE,                                // Main Drawing Layer
                    0,                                            // Reserved
                    0, 0, 0                                        // Layer Masks Ignored
            };

    if (!(hDC = GetDC(hWnd)))                            // Did We Get A Device Context?
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))    // Did Windows Find A Matching Pixel Format?
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    if (!SetPixelFormat(hDC, PixelFormat, &pfd))        // Are We Able To Set The Pixel Format?
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    if (!(hRC = wglCreateContext(hDC)))                // Are We Able To Get A Rendering Context?
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    if (!wglMakeCurrent(hDC, hRC))                    // Try To Activate The Rendering Context
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    ShowWindow(hWnd, SW_SHOW);                        // Show The Window
    SetForegroundWindow(hWnd);                        // Slightly Higher Priority
    SetFocus(hWnd);                                    // Sets Keyboard Focus To The Window
    ReSizeGLScene(width, height);                    // Set Up Our Perspective GL Screen

    if (!InitGL())                                    // Initialize Our Newly Created GL Window
    {
        KillGLWindow();                                // Reset The Display
        MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;                                // Return FALSE
    }

    return TRUE;                                    // Success
}

LRESULT CALLBACK WndProc(HWND hWnd,            // Handle For This Window
                         UINT uMsg,            // Message For This Window
                         WPARAM wParam,            // Additional Message Information
                         LPARAM lParam)            // Additional Message Information
{
    switch (uMsg)                                    // Check For Windows Messages
    {
        case WM_ACTIVATE:                            // Watch For Window Activate Message
        {
            if (!HIWORD(wParam))                    // Check Minimization State
            {
                active = TRUE;                        // Program Is Active
            } else {
                active = FALSE;                        // Program Is No Longer Active
            }

            return 0;                                // Return To The Message Loop
        }

        case WM_SYSCOMMAND:                            // Intercept System Commands
        {
            switch (wParam)                            // Check System Calls
            {
                case SC_SCREENSAVE:                    // Screensaver Trying To Start?
                case SC_MONITORPOWER:                // Monitor Trying To Enter Powersave?
                    return 0;                            // Prevent From Happening
            }
            break;                                    // Exit
        }

        case WM_CLOSE:                                // Did We Receive A Close Message?
        {
            PostQuitMessage(0);                        // Send A Quit Message
            return 0;                                // Jump Back
        }

        case WM_KEYDOWN:                            // Is A Key Being Held Down?
        {
            keys[wParam] = TRUE;                    // If So, Mark It As TRUE
            return 0;                                // Jump Back
        }

        case WM_KEYUP:                                // Has A Key Been Released?
        {
            keys[wParam] = FALSE;                    // If So, Mark It As FALSE
            return 0;                                // Jump Back
        }

        case WM_SIZE:                                // Resize The OpenGL Window
        {
            ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
            return 0;                                // Jump Back
        }
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,            // Instance
                   HINSTANCE hPrevInstance,        // Previous Instance
                   LPSTR lpCmdLine,            // Command Line Parameters
                   int nCmdShow)            // Window Show State
{
    MSG msg;                                    // Windows Message Structure
    BOOL done = FALSE;

    // Create Our OpenGL Window
    if (!CreateGLWindow("NeHe's OpenGL Framework", 640, 480, 32, false)) {
        return 0;                                    // Quit If Window Was Not Created
    }

    while (!done)                                    // Loop That Runs While done=FALSE
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))    // Is There A Message Waiting?
        {
            if (msg.message == WM_QUIT)                // Have We Received A Quit Message?
            {
                done = TRUE;                            // If So done=TRUE
            } else                                    // If Not, Deal With Window Messages
            {
                TranslateMessage(&msg);                // Translate The Message
                DispatchMessage(&msg);                // Dispatch The Message
            }
        } else                                        // If There Are No Messages
        {
            // Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
            if (active)                                // Program Active?
            {
                if (keys[VK_ESCAPE])                // Was ESC Pressed?
                {
                    done = TRUE;                        // ESC Signalled A Quit
                } else                                // Not Time To Quit, Update Screen
                {
                    DrawGLScene();                    // Draw The Scene
                    SwapBuffers(hDC);                // Swap Buffers (Double Buffering)
                }
            }
        }
    }

    // Shutdown
    KillGLWindow();                                    // Kill The Window
    return (msg.wParam);                            // Exit The Program
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)        // Resize And Initialize The GL Window
{
    if (height == 0)                                        // Prevent A Divide By Zero By
    {
        height = 1;                                        // Making Height Equal One
    }

    glViewport(0, 0, width, height);                        // Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glLoadIdentity();                                    // Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(50.0f, (GLfloat) width / (GLfloat) height, 0.1f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
    glLoadIdentity();                                    // Reset The Modelview Matrix
}
