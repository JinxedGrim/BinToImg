#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>
#include "GdiPP.hpp"
#include "WndCreator.hpp"
#include "TerraGL.h"

int sx = GetSystemMetrics(SM_CXSCREEN);
int sy = GetSystemMetrics(SM_CYSCREEN);

void CustomErr(std::string Str)
{
    MessageBoxA(0, Str.c_str(), "Error!", MB_OK);
}

std::string BytesToStr(__int64 ByteCount)
{
    std::string Out = "";

    if (ByteCount < 1024)
    {
        Out = std::to_string((ByteCount)) + " B";
    }
    else if (ByteCount < (1024 * 1024))
    {
        Out = std::to_string((float)((float)ByteCount / (float)(1024))) + " KB";
    }
    else if (ByteCount < (1024 * 1024 * 1024))
    {
        Out = std::to_string((float)((float)ByteCount / (float)(1024 * 1024))) + " MB";
    }
    else
    {
        Out = std::to_string((float)((float)ByteCount / (float)(1024 * 1024 * 1024))) + " GB";
    }

    return Out;
}

void WriteBmpFileHeader(BYTE* Out, __int32 FileSz)
{
    __int32 Reserved_ = 0;

    __int32 DataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    Out[0] = 'B';
    Out[1] = 'M';
    *(__int32*)(Out + 2) = FileSz;
    *(__int32*)(Out + 6) = Reserved_;
    *(__int32*)(Out + 10) = DataOffset;
}

void WriteBmpInfoHeader(BYTE* Out, __int32 ResWidth, __int32 ResHeight)
{
    __int32 Sz = ResWidth * ResHeight * 3;

    __int32 InfoHeaderSz    = 40;//  Off: 0    Static
    __int32 Width    = ResWidth; //  Off: 4    Resolution Width
    __int32 Height   = ResHeight;//  Off: 8    Resolution Height
    short Planes            = 1; //  Off: 12   Static
    short BitsPerPx         = 24;//  Off: 14   BPP 24 = 1 BYTE / channel
    __int32 Compression     = 0; //  Off: 16   0 = BI_RGB 
    __int32 ImageSize       = Sz;//  Off: 20   Compressed Size 0 cause Compression = 0
    __int32 XpixelsPerM     = 0; //  Off: 24   We need no specific res
    __int32 YpixelsPerM     = 0; //  Off: 28   We need no specific res
    __int32 ColorsUsed      = 0; //  Off: 32   Because we dont use a color pallete
    __int32 ImportantColors = 0; //  Off: 36   Because we dont need any 'important' colors

    *(__int32*)(Out + 0) = InfoHeaderSz;
    *(__int32*)(Out + 4) = Width;
    *(__int32*)(Out + 8) = Height;
    *(short*)(Out + 12) = Planes;
    *(short*)(Out + 14) = BitsPerPx;
    *(__int32*)(Out + 16) = Compression;
    *(__int32*)(Out + 20) = ImageSize;
    *(__int32*)(Out + 24) = XpixelsPerM;
    *(__int32*)(Out + 28) = YpixelsPerM;
    *(__int32*)(Out + 32) = ColorsUsed;
    *(__int32*)(Out + 36) = ImportantColors;
}

bool SaveToDisk(std::string FilePath, BYTE* PixelData, __int32 DataLen, __int32 ResWidth, __int32 ResHeight)
{
    __int32 FileSz = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) + (DataLen);

    // I AM MISSING CONVERTING THIS TO BGRBGRBGR
    // ALSO GOTTA DO bottom -> top
    BYTE* FileBuffer = new BYTE[FileSz];

    WriteBmpFileHeader(FileBuffer, FileSz);
    WriteBmpInfoHeader(FileBuffer + sizeof(BITMAPFILEHEADER), ResWidth, ResHeight);
    std::memcpy((FileBuffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)), PixelData, DataLen);

    std::ofstream file(FilePath, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "[!] Failed to open file";
        return false;
    }

    file.write(reinterpret_cast<const char*>(FileBuffer), FileSz);
    file.close();


    return true;
}

bool Draw(GdiPP& Gdi, BYTE* Bytes, __int64 BytesLen, __int64 SrcSzX, __int64 SrcSzY)
{
    if (BytesLen > ((Gdi.ScreenSz.x * Gdi.ScreenSz.y) * 3))
    {
        return false;
    }

    __int64 TotalPx = BytesLen / 3;

    std::cout << "[...] Generating Image" << std::endl;
    for (int j = 0; j < BytesLen; j += 3)
    {
        int DstX = (j / 3) % SrcSzX;
        int DstY = (j / 3) / SrcSzX;

        COLORREF Clr;
        Clr = RGB(Bytes[j], Bytes[j + 1], Bytes[j + 2]);
        Gdi.QuickSetPixel(DstX, DstY, Clr);

        std::cout << "[+] Generated Pixel: " << (j / 3) + 1 << " / " << TotalPx << "       " << "\r";
    }
}

//C:\\Users\\griff\\Desktop\\My Stuff\\My Programming\\Pong\\x64\\Release\\Pong.exe
//C:\\Users\\griff\\Desktop\\TestEncode.txt

// TODO: Chunk Data to encode large files

BYTE* EncodeData(BYTE* Bytes, __int64 BytesLen, __int64 BufferSz)
{
    BYTE* Buffer = new BYTE[BufferSz];
    std::memset(Buffer, 0, BufferSz);

    std::memcpy(Buffer, Bytes, BufferSz);
    return Buffer;
}

BYTE* DecodeData(BYTE* PixelData, __int64 BytesLen)
{
    return PixelData;
}

int main()
{
    WndCreatorW Cmd = GetConsoleWindow();

    // Create Brush and Pen
    HBRUSH ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

    // Create Overlay
    WndCreatorW Window = WndCreatorW(CS_OWNDC, L"BinToImg", L"BinToImg", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, 0, WndModes::BorderLess | WndModes::ClipChildren, 0, 0, sx, sy);

    Window.Hide();

    //Window.SetLayeredAttributes(RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Create gdi Object
    GdiPP gdi = GdiPP(Window.Wnd, true);
    gdi.ErrorHandler = CustomErr;

    PenPP CurrentPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

    // Select Pen And Brushb
    gdi.ChangeBrush(ClearBrush);
    gdi.ChangePen(CurrentPen);

    // Init Variables
    MSG msg = { 0 };
    POINT p;

    int sx = Window.GetClientArea().Width;
    int sy = Window.GetClientArea().Height;

    std::string Path = "";
    int ArWidth = 0;
    int ArHeight = 0;
    __int64 NewWidthPx = 0;
    __int64 NewHeightPx = 0;
    bool DoPreview = true;


    system("title BinToImg By JinxedGrim");

    while (!GetAsyncKeyState(VK_RETURN))
    {
        DoPreview = true;
        PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);
        // Translate and Dispatch message to WindowProc
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Check Msg
        if (msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY)
        {
            break;
        }

        system("cls");
        std::cin.clear();

        Window.Hide();
        Cmd.Show();

        std::cout << "BinToImg By JinxedGrim (Press back space to exit image preview)" << std::endl;
        std::cout << "The max size per file that can be previewed is: " << BytesToStr(sx * sy * 3) << " However files can be encoded as several images" << std::endl;
        std::cout << "Enter the path of the file you want to encode to a bitmap (format: C:\\\\SomeFolder\\\\SomeFile): " << std::endl << std::endl << "[?] Path: ";
        std::getline(std::cin, Path);

        std::ifstream FileStream(Path, std::ios::binary);

        if (!FileStream.is_open())
        {
            MessageBoxA(0, "File No Exist Idiot", "YOU'RE DUMB BRO", MB_OK);
            continue;
        }
        std::cout << "[+] Opened File" << std::endl;

        std::cout << "[...] Reading File Data" << std::endl;
        std::vector<char> bytes((std::istreambuf_iterator<char>(FileStream)), (std::istreambuf_iterator<char>()));
        FileStream.close();
        std::cout << "[+] File is: " << BytesToStr(bytes.size()) << std::endl;

        std::cout << "[?] Pick an aspect ratio (16:9; 4:3; etc):" << std::endl;

        std::cout << "[?] Width: ";
        std::cin >> ArWidth;
        std::cout << "[?] Height: ";
        std::cin >> ArHeight;

        NewWidthPx = PixelRoundCeil(sqrt((float)bytes.size() / (3 * (ArWidth / ArHeight))));
        NewHeightPx = PixelRoundCeil((ArWidth / ArHeight) * NewWidthPx);
        __int32 Stride = ((NewWidthPx * 3) + 3) & ~3;
        NewWidthPx = PixelRoundCeil(Stride / 3);
        __int64 FullSzBytes = NewHeightPx * NewWidthPx * 3;

        std::cout << "[+] Resolution: " << NewWidthPx << "x" << NewHeightPx << std::endl;

        //TODO: Do some file chunking here 
        // maybe count how many chunks are needed then generate an array of byte pointers which go to the arrays we need?

        if (FullSzBytes > (sx * sy * 3))
        {
            std::cerr << "[!] File is to large to preview!" << std::endl;
            std::cout << "[+] File can be split into: " << (FullSzBytes / 3.0f) / (sx * sy * 3) << " Images" << std::endl << std::endl;
            std::cout << "[?]" << std::endl;
            DoPreview = false;
        }


        gdi.Clear();

        BYTE* Data = EncodeData((BYTE*)bytes.data(), bytes.size(), FullSzBytes);

        if (DoPreview)
        {
            Draw(gdi, Data, NewWidthPx * NewHeightPx * 3, NewWidthPx, NewHeightPx);

            Window.Show();
            Cmd.Hide();

            gdi.DrawDoubleBuffer();

            while (!GetAsyncKeyState(VK_BACK))
            {
                Sleep(150);
            }
        }

        SaveToDisk("Output1.bmp", Data, bytes.size(), NewWidthPx, NewHeightPx);

        std::cout << "[...] Testing Decode" << std::endl;

        BYTE* FileBytes = DecodeData(Data, bytes.size());

        std::ofstream file("Output", std::ios::binary);

        if (!file.is_open()) 
        {
            std::cerr << "[!] Failed to open file";
            continue;
        }

        file.write(reinterpret_cast<const char*>(Data), bytes.size());
        file.close();

        delete[] Data;
    }

    Window.Destroy();

    return 0;
}