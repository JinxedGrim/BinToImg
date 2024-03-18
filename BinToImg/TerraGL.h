#include <Windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <dwrite.h>


#ifdef _MSC_VER
#pragma comment(lib, "d2d1") // -ld2d1
#pragma comment(lib, "dwrite") // -ldwrite
#pragma comment(lib, "windowscodecs")
#endif

#define CalcCenterAlignTextLayout(x, y, Width, Height) D2D1::RectF(x - Width, y - Height, x + Width, y + Height) 

class TerraGL
{
    public:

    HWND Hwnd = NULL;

    // Window info
    RECT ClientRect = {};
    D2D1_SIZE_U WndSz = {};

    ID2D1Factory* D2D1Factory = nullptr;

    // Rendering stuff
    ID2D1HwndRenderTarget* RenderTarget = nullptr;
    ID2D1SolidColorBrush* Brush = nullptr;
    bool CurrentlyDrawing = false;

    // PixelBuffer stuff
    D2D1_BITMAP_PROPERTIES BitmapProperties;
    ID2D1Bitmap* Bitmap = nullptr;
    BYTE* PixelBuffer = nullptr;
    int Stride = 0;
    int PixelBufferWidth = 0;
    int PixelBufferHeight = 0;

    // Dwrite
    IDWriteFactory* DwriteFactory = nullptr;
    IDWriteTextFormat* TextFormat = nullptr;

    bool UpdateClientRgn(bool UpdatePixelBuffer = false)
    {
        bool Stat = GetClientRect(this->Hwnd, &this->ClientRect);

        this->WndSz = D2D1::SizeU(ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);

        HRESULT Hr = S_OK;

        Hr = D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(this->Hwnd, this->WndSz), &RenderTarget);

        if (UpdatePixelBuffer == true)
        {
            this->SetupPixelBuffer((ClientRect.right - ClientRect.left), (ClientRect.bottom - ClientRect.top), 4);
        }

        return Stat == true && SUCCEEDED(Hr);
    }

    void SetBitmapProperties(const D2D1_PIXEL_FORMAT& PixelFormat, float DpiX = 96.0f, float DpiY = 96.0f)
    {
        this->BitmapProperties = D2D1::BitmapProperties(PixelFormat, DpiX, DpiY);
    }

    void SetupPixelBuffer(int Width, int Height, int BytesPerPixel)
    {
        this->PixelBufferWidth = Width;
        this->PixelBufferHeight = Height;

        if (PixelBuffer != nullptr)
            delete[] PixelBuffer;

        this->Stride = ((this->PixelBufferWidth * BytesPerPixel + 3) / 4) * 4; // Calculate stride

        PixelBuffer = new BYTE[this->Stride * this->PixelBufferHeight];
    }

    void __inline QuickSetPixel()
    {

    }

    TerraGL(HWND _Wnd)
    {
        this->Hwnd = _Wnd;
        this->CurrentlyDrawing = false;

        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);

        if (this->D2D1Factory == nullptr)
        {
            return;
        }

        if (!this->UpdateClientRgn(true))
        {
            return;
        }

        this->SetBitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_STRAIGHT));

        HRESULT Hr = S_OK; 
        Hr = this->RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &Brush);

        if (!SUCCEEDED(Hr))
        {
            return;
        }

        Hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&this->DwriteFactory));

        if (!SUCCEEDED(Hr))
        {
            return;
        }

        Hr = this->DwriteFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us", &this->TextFormat);

        if (!SUCCEEDED(Hr))
        {
            return;
        }

        TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    bool DrawPixelBuffer()
    {
        HRESULT Hr = S_OK;

        if (SUCCEEDED(Hr))
        {
            if (this->Bitmap != nullptr)
            {
                this->Bitmap->Release();
            }

            Hr = this->RenderTarget->CreateBitmap(this->WndSz, (void*)this->PixelBuffer, (UINT32)this->Stride, this->BitmapProperties, &Bitmap);
        }

        return SUCCEEDED(Hr);
    }

    __inline void SetBrushColor(const D2D1::ColorF& Color)
    {
        if (this->Brush != nullptr)
        {
            this->Brush->SetColor(Color);
        }
        else
        {
            this->RenderTarget->CreateSolidColorBrush(Color, &this->Brush);
        }
    }

    __inline void BeginDraw()
    {
        this->CurrentlyDrawing = true;
        this->RenderTarget->BeginDraw();
    }

    __inline void Clear(const D2D1_COLOR_F* ClearColor = (const D2D1_COLOR_F*)0)
    {
        if (this->CurrentlyDrawing == false)
            this->BeginDraw();

        this->RenderTarget->Clear(ClearColor);

        if (this->CurrentlyDrawing == false)
            this->EndDraw();
    }

    __inline void EndDraw()
    {
        this->RenderTarget->EndDraw();
        this->CurrentlyDrawing = false;
    }

    ~TerraGL()
    {
        if (this->Brush)
        {
            Brush->Release();
        }

        if(this->Bitmap)
        {
            Bitmap->Release();
        }

        if(this->PixelBuffer)
        {
            delete[] PixelBuffer;
        }

        if (this->TextFormat)
        {
            this->TextFormat->Release();
        }
        
        if (this->DwriteFactory)
        {
            this->DwriteFactory->Release();
        }

        if(this->RenderTarget)
        {
            RenderTarget->Release();
        }

        if(this->D2D1Factory)
        {
            D2D1Factory->Release();
        }
    }
};
