#ifdef _WIN32

#include "windowsShellThumbProvider.h"

#include <windows.h>
#include <shobjidl.h>   // IShellItem, IShellItemImageFactory
#include <objbase.h>

// --- 小工具：RAII 的 COM 初始化作用域 ---
// Define a tool class to initialize COM within its scope
//COM: Component Object Model, Windows API
//HRESULT: return type of handle request
class ComInitScope
{
public:
    ComInitScope()
        : m_initialized(false)
    {
        // 单线程模式或 UI 线程建议用 COINIT_APARTMENTTHREADED
        /* 
        //CoInitializeEx: function to initialize COM
        2nd argument represents 2 modes of it. 
        //COINIT_APARTMENTTHHREADED: single threaded apartment mode
        //COINIT_MULTITHREADED: multi threaded apartment mode
        For frontend/UI/single-threaded uses, use S mode
        For backend batch pipeline, use M mode. 
        Here importing thumbnails is mainly called by QML UI. Thus S mode. 
        */
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr)) {
            m_initialized = true;
        } else if (hr == RPC_E_CHANGED_MODE) {
            //if already initialized with the other mode, proceed
            m_initialized = true;
        } else {
            m_initialized = false;
        }
    }

    ~ComInitScope()
    {
        if (m_initialized) {
            CoUninitialize(); //turn off
        }
    }

    bool ok() const { return m_initialized; }

private:
    bool m_initialized;
};


// --- Tool Function：HBITMAP -> QImage ---
//HBITMAP: a handle to access the bit image stored in system interface.
static QImage hbitmapToQImage(HBITMAP hbm)
{
    if (!hbm)
        return {};

    BITMAP bm;
    if (!GetObject(hbm, sizeof(bm), &bm)) { //get bm from hbm
        return {}; //if fail return null
    }

    // request BITMAPINFO in 32 bit ARGB
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi)); //empty bmi object by setting the memory to zero
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = bm.bmWidth;
    bmi.bmiHeader.biHeight      = -bm.bmHeight; // negative means top-down bitmap
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    QImage image(bm.bmWidth, bm.bmHeight, QImage::Format_ARGB32);
    if (image.isNull())
        return {};

    HDC hdc = GetDC(nullptr); //DC: device context
    if (!hdc) {
        return {};
    }
    //DIB: device independent bitmap
    if (!GetDIBits(hdc,//device context
                   hbm,//bitmap handle
                   0, //starts from line 0
                   static_cast<UINT>(bm.bmHeight),//line height to scan
                   image.bits(),//directly write to QImage memory
                   &bmi,
                   DIB_RGB_COLORS)) { //execute, if fail, return null
        ReleaseDC(nullptr, hdc);
        return {};
    }

    ReleaseDC(nullptr, hdc);
    return image;
}

// --- 核心实现 ---
//Core implementation:from source file path, requested size, and cache dir to ThumbImage object linked to cached file. 
QString WindowsShellThumbProvider::makeThumbnail(const QString &filePath, 
    const QSize &targetSize, const QString &cacheDir)
{
    QString result;

    if (filePath.isEmpty()) { //check if source file wrong, return null
        return QString();
    }

    //Initialize COM
    // COM must run on the same thread
    //Initialize COM for every function call to avoid thread error
    ComInitScope com;
    if (!com.ok()) {
        qWarning() << "WindowsShellThumbProvider: COM init failed";
        return QString();
    }

    //Path conversion
    const QString nativePath = QDir::toNativeSeparators(filePath);//convert to native path for the OS
    const std::wstring wPath = nativePath.toStdWString();

    IShellItem *psi = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(wPath.c_str(), nullptr,
                                             IID_PPV_ARGS(&psi)); //create shell item by parsing path and record result to hr
    if (FAILED(hr) || !psi) {
        qWarning() << "WindowsShellThumbProvider: SHCreateItemFromParsingName failed"
                   << hr;
        return QString();
    }

    IShellItemImageFactory *factory = nullptr;
    hr = psi->QueryInterface(IID_PPV_ARGS(&factory)); //
    psi->Release();

    if (FAILED(hr) || !factory) {
        qWarning() << "WindowsShellThumbProvider: QueryInterface(IShellItemImageFactory) failed"
                   << hr;
        return QString();
    }

    //target size
    SIZE size;
    size.cx = targetSize.isValid() ? targetSize.width() : 256;
    size.cy = targetSize.isValid() ? targetSize.height() : 256;

    HBITMAP hBmp = nullptr;

    // flags 控制行为：
    //  - SIIGBF_RESIZETOFIT      : 缩放到给定 size
    //  - SIIGBF_BIGGERSIZEOK     : 如果缓存中有更大图，允许返回更大
    //  - SIIGBF_ICONONLY         : 某些非图片类型返回图标
    SIIGBF flags = SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK;

    hr = factory->GetImage(size, flags, &hBmp);
    factory->Release();

    if (FAILED(hr) || !hBmp) { //in either case hBmp would not exist
        qWarning() << "WindowsShellThumbProvider: GetImage failed" << hr;
        return QString();
    }

    QImage img = hbitmapToQImage(hBmp); //hbitmap to QImange conversion
    DeleteObject(hBmp);

    if (img.isNull()) {
        qWarning() << "WindowsShellThumbProvider: hbitmapToQImage failed";
        return QString();
    }

    // 缩略图缓存目录
    //const QString cacheDirPath =
    //    QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    //    + "/zviewer_thumbs";

    /*
    //portable version cache location
    const QString cacheDirPath =
        QCoreApplication::applicationDirPath() + "/cache/zviewer_thumbs";

    QDir().mkpath(cacheDirPath);
    */

    // Ensure cache directory exists
    QDir dir(cacheDir);
    if (!dir.exists() && !dir.mkpath(".")) { //if dir not exist, make dir path, if fail, return null
        qWarning() << "WindowsShellThumbProvider: cannot create cache directory:" << cacheDir;
        return QString();
    }

    //use hash hex to name cache file
    const QString thumbFileName = hashFileName(filePath) + ".png";
    result = dir.filePath(thumbFileName);

    if (!img.save(result, "PNG")) { //save cache file, if fail, return null. 
        qWarning() << "WindowsShellThumbProvider: save thumbnail failed" << result;
        return QString();
    }
    return result; 
}

#endif // _WIN32
