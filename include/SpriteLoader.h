 #ifndef SPRITELOADER_H
#define SPRITELOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "graphics.h"
#include <cstdlib>   // malloc, free
#include <iostream>  // std::cout

typedef unsigned char u8;

#ifndef COLOR
#define COLOR(r,g,b) ((int)(((r)&0xFF)|(((g)&0xFF)<<8)|(((b)&0xFF)<<16)))
#endif

class SpriteLoader {
private:
    int sheetH;
    int sheetW;
    int frameH;
    int frameW;
    int NumFrame;

    inline static unsigned short rd16(std::ifstream &f)
    {
        u8 b[2]; f.read((char*)b,2);
        return (unsigned short)(b[0] | (b[1] << 8));
    }

    inline static unsigned int rd32(std::ifstream &f)
    {
        u8 b[4]; f.read((char*)b,4);
        return (unsigned int)(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
    }

    inline static void* GrabFrame2(int l,int t,int r,int b)
    {
        unsigned sz = imagesize(l,t,r,b);
        void* buf = std::malloc(sz);
        if(!buf) throw std::runtime_error("malloc gagal frame");
        getimage(l,t,r,b,buf);
        return buf;
    }

    // =========================================================
    // NEW: Baca header BMP saja untuk mendapatkan W,H & bpp
    //      (tanpa menggambar).
    // =========================================================
    inline static void ProbeBMPSize_(const std::string& path, int& W, int& H, unsigned short& bpp)
    {
        std::ifstream fin(path.c_str(), std::ios::binary);
        if(!fin) throw std::runtime_error("Tidak bisa buka file: " + path);

        char m[2]; fin.read(m,2);
        if(m[0]!='B'||m[1]!='M') throw std::runtime_error("Bukan BMP");

        (void)rd32(fin); (void)rd16(fin); (void)rd16(fin);
        (void)rd32(fin);                       // dataOffset
        unsigned dib = rd32(fin);
        if(dib < 40) throw std::runtime_error("Header BMP tidak valid");

        int w  = (int)rd32(fin);
        int hs = (int)rd32(fin);
        bool topDown = (hs<0); (void)topDown;
        int h = (hs<0 ? -hs : hs);

        (void)rd16(fin);
        unsigned short BPP = rd16(fin);

        W = w; H = h; bpp = BPP;
    }

    // =========================================================
    // NEW: Buka jendela WinBGIm off-screen dan aktifkan double
    //      buffering, lalu set ke back-buffer sebagai active page.
    //      Window ini tidak terlihat karena posisinya jauh di luar
    //      layar.
    // =========================================================
    inline static void EnsureHiddenWindow_(int W, int H)
    {
        // Inisialisasi window off-screen.
        // Signature WinBGIm: initwindow(width, height, title, left, top, dbflag=true, closeflag=true)
        // Taruh di koordinat negatif besar supaya tak terlihat.
        initwindow(W, H, "SpriteLoaderHidden", -10000, -10000, /*dbflag*/true, /*closeflag*/true);

        // Pastikan kita menggambar di back-buffer (page 1), dan yang terlihat adalah page 0.
        // Karena window off-screen, sebenarnya tak terlihat juga, tapi ini menjamin "benar-benar"
        // tidak di-visualisasikan.
        setactivepage(1);
        setvisualpage(0);

        // Bersihkan back-buffer.
        setbkcolor(BLACK);
        cleardevice();
    }

public:
    inline SpriteLoader()
    : sheetH(0), sheetW(0), frameH(0), frameW(0)
    {}

    inline int GetSheetW() const { return sheetW; }
    inline int GetSheetH() const { return sheetH; }
    inline int GetFrameW() const { return frameW; }
    inline int GetFrameH() const { return frameH; }
    inline int GetFrameNum() const { return NumFrame; }

    inline void LoadBMP(const std::string &path)
    {
        std::ifstream fin(path.c_str(), std::ios::binary);
        if(!fin) throw std::runtime_error("Tidak bisa buka file: " + path);

        char m[2]; fin.read(m,2);
        if(m[0]!='B'||m[1]!='M') throw std::runtime_error("Bukan BMP");

        (void)rd32(fin); (void)rd16(fin); (void)rd16(fin);
        unsigned dataOffset = rd32(fin);
        unsigned dib = rd32(fin);
        if(dib < 40) throw std::runtime_error("Header BMP tidak valid");

        int W  = (int)rd32(fin);
        int Hs = (int)rd32(fin);
        bool topDown = (Hs<0);
        int H = (Hs<0 ? -Hs : Hs);

        (void)rd16(fin);
        unsigned short bpp = rd16(fin);
        unsigned comp = rd32(fin);
        if(comp!=0) throw std::runtime_error("BMP kompresi tidak didukung");

        (void)rd32(fin); (void)rd32(fin); (void)rd32(fin);
        unsigned used = rd32(fin);
        (void)rd32(fin);

        std::vector<u8> pal;
        if(bpp==8){
            unsigned cnt = used ? used : 256;
            pal.resize(cnt*4);
            fin.read((char*)&pal[0], cnt*4);
        }

        fin.seekg((std::streamoff)dataOffset, std::ios::beg);

        if(bpp==24){
            int rowBytes = W*3;
            int pad = (4-(rowBytes%4))%4;
            std::vector<u8> row(rowBytes);

            for(int y=0;y<H;y++){
                fin.read((char*)&row[0], rowBytes);
                if(pad) fin.seekg(pad,std::ios::cur);
                int yy = topDown ? y : (H-1-y);
                for(int x=0;x<W;x++){
                    u8 B=row[3*x], G=row[3*x+1], R=row[3*x+2];
                    putpixel(x,yy,COLOR(R,G,B));
                }
            }
        }
        else if(bpp==32){
            std::vector<u8> row(W*4);
            for(int y=0;y<H;y++){
                fin.read((char*)&row[0], row.size());
                int yy = topDown ? y : (H-1-y);
                for(int x=0;x<W;x++){
                    u8 B=row[4*x],G=row[4*x+1],R=row[4*x+2];
                    putpixel(x,yy,COLOR(R,G,B));
                }
            }
        }
        else if(bpp==8){
            int pad = (4-(W%4))%4;
            std::vector<u8> row(W);
            for(int y=0;y<H;y++){
                fin.read((char*)&row[0],W);
                if(pad) fin.seekg(pad,std::ios::cur);
                int yy = topDown?y:(H-1-y);
                for(int x=0;x<W;x++){
                    size_t p=row[x]*4;
                    putpixel(x,yy,COLOR(pal[p+2],pal[p+1],pal[p]));
                }
            }
        }
        else{
            throw std::runtime_error("BMP hanya 8/24/32-bit");
        }

        sheetW=W;
        sheetH=H;
    }

    // =========================================================
    // NEW: LoadStrip tersembunyi — membuka window off-screen,
    //      menggambar ke back-buffer, ambil frame, tutup window.
    //      Tidak ada tampilan ke layar sama sekali.
    // =========================================================
    inline std::vector<void*> LoadStrip(const std::string& filepath, int n)
    {
        if(n<=0) throw std::runtime_error("LoadStrip: n harus > 0");

        // 1) Pastikan ukuran BMP
        int W=0, H=0; unsigned short bpp=0;
        ProbeBMPSize_(filepath, W, H, bpp);

        // 2) Inisialisasi jendela tersembunyi off-screen
        EnsureHiddenWindow_(W, H);

        // 3) Gambar BMP ke page aktif (back buffer) — tidak terlihat
        LoadBMP(filepath);

        if(sheetW<=0 || sheetH<=0) {
            closegraph();
            throw std::runtime_error("LoadStrip: sheet gagal terbaca");
        }

        // 4) Hitung ukuran frame dan potong
        frameW = sheetW / n;
        frameH = sheetH;
        NumFrame = n;

        std::vector<void*> frames(n);
        for(int i=0;i<n;i++){
            int l=i*frameW, t=0;
            frames[i]=GrabFrame2(l,t,l+frameW-1,frameH-1);
        }

        // 5) Tutup jendela tersembunyi (buffers tetap aman di RAM)
        closegraph();

        return frames;
    }

    inline std::vector<void*> LoadGrid(int rows,int cols)
    {
        if(rows<=0||cols<=0||sheetW<=0||sheetH<=0) throw std::runtime_error("LoadGrid: parameter/sheet invalid");
        frameW = sheetW / cols;
        frameH = sheetH / rows;

        std::vector<void*> frames;
        frames.reserve(rows*cols);
     NumFrame = rows*cols;

        for(int r=0;r<rows;r++)
            for(int c=0;c<cols;c++)
                frames.push_back(
                    GrabFrame2(c*frameW,r*frameH,c*frameW+frameW-1,r*frameH+frameH-1)
                );
        return frames;
    }

    inline std::vector<void*> LoadGridArea(
        int rows,int cols,
        int startX,int startY,
        int endX,int endY
    )
    {
        if(rows<=0||cols<=0) throw std::runtime_error("LoadGridArea: rows/cols invalid");
        if(startX<0||startY<0||endX<=startX||endY<=startY) throw std::runtime_error("LoadGridArea: area invalid");
        if(sheetW<=0||sheetH<=0) throw std::runtime_error("LoadGridArea: sheet belum di-load");
        if(endX>sheetW||endY>sheetH) throw std::runtime_error("LoadGridArea: area di luar sheet");

        int areaW = endX - startX;
        int areaH = endY - startY;

        frameW = areaW / cols;
        frameH = areaH / rows;

        std::vector<void*> frames;
        frames.reserve(rows*cols);
        NumFrame=0;

        for(int r=0;r<rows;r++)
            for(int c=0;c<cols;c++){
                NumFrame++;
                int l = startX + c*frameW;
                int t = startY + r*frameH;
                frames.push_back(GrabFrame2(l,t,l+frameW-1,t+frameH-1));
            }
        return frames;
    }

    inline std::vector<void*> LoadFlex(int rows,int cols,int sr,int sc,int dr,int dc)
    {
        if(rows<=0||cols<=0||sheetW<=0||sheetH<=0) throw std::runtime_error("LoadFlex: parameter/sheet invalid");
        if(dr==0 && dc==0) throw std::runtime_error("LoadFlex: step nol");

        frameW = sheetW / cols;
        frameH = sheetH / rows;

        std::vector<void*> frames;
        int r=sr,c=sc,guard=rows*cols+10;
NumFrame=0;
        while(r>=0 && r<rows && c>=0 && c<cols && guard--){
            frames.push_back(
                GrabFrame2(c*frameW,r*frameH,c*frameW+frameW-1,r*frameH+frameH-1)
            );
                NumFrame++;
            r+=dr; c+=dc;
        }
        return frames;
    }

    inline static void Free(std::vector<void*> &frames)
    {
        std::vector<void*>::iterator it=frames.begin();
        for(;it!=frames.end();++it)
            if(*it) std::free(*it);
        frames.clear();
    }

    inline static void Help()
    {
        std::cout
        << "SpriteLoader API (header-only)\n"
        << "---------------------------------\n"
        << "Konstruktor:\n"
        << "  SpriteLoader()\n\n"

        << "Getter ukuran:\n"
        << "  int GetSheetW() const\n"
        << "  int GetSheetH() const\n"
        << "  int GetFrameW() const\n"
        << "  int GetFrameH() const\n\n"

        << "Loader BMP:\n"
        << "  void LoadBMP(const std::string& path)\n"
        << "    - Menggambar BMP ke (0,0) pada page aktif dan menyimpan sheetW/sheetH.\n\n"

        << "Loader Sprite (tak terlihat):\n"
        << "  std::vector<void*> LoadStrip(const std::string& filepath, int n)\n"
        << "    - Membuka window off-screen, menggambar ke back-buffer, ambil n frame,\n"
        << "      lalu menutup window. Tidak ada tampilan ke layar.\n\n"

        << "  std::vector<void*> LoadGrid(int rows, int cols)\n"
        << "  std::vector<void*> LoadGridArea(int rows, int cols, startX,startY,endX,endY)\n"
        << "  std::vector<void*> LoadFlex(int rows, int cols, sr,sc, dr,dc)\n\n"

        << "Manajemen memori:\n"
        << "  static void Free(std::vector<void*>& frames)\n\n"

        << "Catatan:\n"
        << "  * Buffer hasil GrabFrame2 dapat langsung dipakai putimage(x,y,frame,COPY_PUT).\n"
        << "  * LoadStrip bekerja sepenuhnya di belakang layar (off-screen + back-buffer).\n"
        << "  * Hindari closegraph sebelum Anda selesai menggunakan frame (dalam sesi yang sama\n"
        << "    sebaiknya tetap menggunakan WinBGIm; format buffer mengikuti driver yang sama).\n"
        << std::endl;
    }
};

#endif
