#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <vector>
#include <cstring>

#define IDB_BANNER 201
#define IDI_CUSTOM 202
#define IDR_7ZR 203
#define IDB_LOGO 204

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "msimg32.lib")

#define ID_PLAY 1001
#define ID_OPEN 1002
#define ID_VALIDATE 1003
#define ID_EXIT 1004
#define ID_FOLDER 1005
#define ID_YOUTUBE 1007
#define ID_DISCORD 1008

HINSTANCE gInstance;
HWND gStatus, gInfo;
HBITMAP gBanner=nullptr;
HBITMAP gLogo=nullptr;
const COLORREF PINK = RGB(255,79,163), DARK = RGB(19,13,22), PANEL = RGB(38,23,35), MUTED = RGB(205,178,199), WHITE = RGB(255,247,252);

void SetStatus(const std::wstring& text){ SetWindowTextW(gStatus, text.c_str()); }

std::wstring EnvironmentReport(){
  SYSTEM_INFO si{}; GetSystemInfo(&si);
  MEMORYSTATUSEX mem{}; mem.dwLength=sizeof(mem); GlobalMemoryStatusEx(&mem);
  ULARGE_INTEGER freeBytes{}, totalBytes{}, totalFree{}; GetDiskFreeSpaceExW(nullptr,&freeBytes,&totalBytes,&totalFree);
  double freeGB=(double)freeBytes.QuadPart/1073741824.0;
  std::wstringstream s;
  wchar_t cpu[256]{}; HKEY key; if(RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",0,KEY_READ,&key)==ERROR_SUCCESS){ DWORD size=sizeof(cpu); RegQueryValueExW(key,L"ProcessorNameString",nullptr,nullptr,(LPBYTE)cpu,&size); RegCloseKey(key); }
  s<<L"CPU: "<<(cpu[0]?cpu:L"não informado")<<L"\r\n"
   <<L"Arquitetura: "<<(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64?L"x64":si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_ARM64?L"ARM64":L"compatível")<<L"   •   Núcleos: "<<si.dwNumberOfProcessors<<L"\r\n"
   <<L"RAM total: "<<(mem.ullTotalPhys/1073741824)<<L" GB   •   Uso: "<<mem.dwMemoryLoad<<L"%   •   Livre: "<<(mem.ullAvailPhys/1073741824)<<L" GB\r\n"
   <<L"Disco C livre: "<<(int)freeGB<<L" GB   •   Container: C:\\Grand Theft Auto V Lite\\Download\r\n"
   <<(freeGB>=50?L"✓ Espaço recomendado disponível":L"! Recomendado liberar espaço antes de criar o container");
  return s.str();
}

std::wstring PickFile(){
  OPENFILENAMEW ofn{}; wchar_t path[MAX_PATH]=L""; ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=GetActiveWindow(); ofn.lpstrFile=path; ofn.nMaxFile=MAX_PATH;
  ofn.lpstrFilter=L"Programas e atalhos (*.exe;*.lnk)\0*.exe;*.lnk\0Todos os arquivos (*.*)\0*.*\0"; ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
  return GetOpenFileNameW(&ofn)?std::wstring(path):L"";
}

std::wstring LauncherDirectory(){
  wchar_t path[MAX_PATH]{}; GetModuleFileNameW(nullptr,path,MAX_PATH);
  std::wstring p(path); size_t slash=p.find_last_of(L"\\/"); return slash==std::wstring::npos?L".":p.substr(0,slash);
}

std::wstring GameDirectory(){
  // Caminho próprio e reversível para o usuário colocar arquivos que possui legalmente.
  return L"C:\\Grand Theft Auto V Lite\\Download";
}

bool IsFile(const std::wstring& path){ return GetFileAttributesW(path.c_str())!=INVALID_FILE_ATTRIBUTES && !(GetFileAttributesW(path.c_str())&FILE_ATTRIBUTE_DIRECTORY); }

std::wstring FindExtractor(){
  std::wstring candidates[]={
    LauncherDirectory()+L"\\7z.exe",
    GameDirectory()+L"\\7z.exe",
    L"C:\\Program Files\\7-Zip\\7z.exe",
    L"C:\\Program Files (x86)\\7-Zip\\7z.exe",
    L"C:\\Windows\\System32\\7z.exe"
  };
  for(const auto& p:candidates) if(IsFile(p)) return p;
  return L"";
}

std::wstring Embedded7zr(){
  std::wstring out=LauncherDirectory()+L"\\7zr.exe";
  if(IsFile(out)) return out;
  HRSRC res=FindResourceW(gInstance,MAKEINTRESOURCEW(IDR_7ZR),RT_RCDATA); if(!res) return L"";
  HGLOBAL loaded=LoadResource(gInstance,res); DWORD size=SizeofResource(gInstance,res); void* data=LockResource(loaded); if(!data||!size) return L"";
  HANDLE f=CreateFileW(out.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr); if(f==INVALID_HANDLE_VALUE) return L"";
  DWORD written=0; BOOL ok=WriteFile(f,data,size,&written,nullptr); CloseHandle(f); return ok&&written==size?out:L"";
}

std::wstring FindFirstPart(){
  WIN32_FIND_DATAW data{};
  HANDLE zero=FindFirstFileW((GameDirectory()+L"\\*.000").c_str(),&data);
  if(zero!=INVALID_HANDLE_VALUE){ std::wstring result=GameDirectory()+L"\\"+data.cFileName; FindClose(zero); return result; }
  HANDLE h=FindFirstFileW((GameDirectory()+L"\\*.001").c_str(),&data);
  if(h==INVALID_HANDLE_VALUE) return L"";
  std::wstring result=GameDirectory()+L"\\"+data.cFileName; FindClose(h); return result;
}

bool AllPartsPresent(const std::wstring& first, int count, std::wstring& missing){
  size_t dot=first.rfind(L".000"); int start=0; if(dot==std::wstring::npos){ dot=first.rfind(L".001"); start=1; } if(dot==std::wstring::npos) return false;
  std::wstring stem=first.substr(0,dot);
  for(int i=start;i<=count;i++){
    wchar_t suffix[8]; swprintf_s(suffix,L".%03d",i);
    if(!IsFile(stem+suffix)){ missing=stem+suffix; return false; }
  }
  return true;
}

bool ExtractLocalArchive(const std::wstring& first){
  std::wstring seven=FindExtractor();
  if(seven.empty()) seven=Embedded7zr();
  if(seven.empty()){
    MessageBoxW(GetActiveWindow(),L"As partes foram encontradas, mas 7z.exe não está disponível. Coloque 7z.exe junto do launcher ou instale o 7-Zip dentro do ambiente.",L"Extrator não encontrado",MB_OK|MB_ICONERROR);
    SetStatus(L"7z.exe não encontrado."); return false;
  }
  SetStatus(L"Extraindo partes locais… não feche o launcher.");
  std::wstring command=L"\""+seven+L"\" x \""+first+L"\" -o\""+GameDirectory()+L"\" -y";
  std::vector<wchar_t> mutableCommand(command.begin(),command.end()); mutableCommand.push_back(L'\0');
  STARTUPINFOW si{}; si.cb=sizeof(si); PROCESS_INFORMATION pi{};
  BOOL ok=CreateProcessW(nullptr,mutableCommand.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,GameDirectory().c_str(),&si,&pi);
  if(!ok){ SetStatus(L"Não foi possível iniciar o extrator."); MessageBoxW(GetActiveWindow(),L"O Windows/Winlator não conseguiu iniciar 7z.exe.",L"Erro de extração",MB_OK|MB_ICONERROR); return false; }
  WaitForSingleObject(pi.hProcess,INFINITE); DWORD exitCode=1; GetExitCodeProcess(pi.hProcess,&exitCode); CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
  if(exitCode!=0){ SetStatus(L"A extração terminou com erro."); MessageBoxW(GetActiveWindow(),L"A extração falhou. Verifique se as seis partes estão completas e não foram renomeadas.",L"Erro de extração",MB_OK|MB_ICONERROR); return false; }
  SetStatus(L"Extração concluída. Verificando PlayGTA.exe…"); return true;
}

bool EnsureGameDirectory(){
  std::wstring dir=GameDirectory();
  if(CreateDirectoryW(L"C:\\Grand Theft Auto V Lite",nullptr)==0 && GetLastError()!=ERROR_ALREADY_EXISTS) return false;
  if(CreateDirectoryW(dir.c_str(),nullptr)==0 && GetLastError()!=ERROR_ALREADY_EXISTS) return false;
  return true;
}

std::wstring FindPlayGTA(){
  std::wstring candidates[]={
    LauncherDirectory()+L"\\PlayGTA5.exe",
    LauncherDirectory()+L"\\PlayGTA.exe",
    GameDirectory()+L"\\PlayGTA5.exe",
    GameDirectory()+L"\\PlayGTA.exe",
    L"C:\\Grand Theft Auto V Lite\\PlayGTA5.exe",
    L"C:\\Grand Theft Auto V Lite\\PlayGTA.exe",
    L"C:\\Grand Theft Auto V Lite\\Download\\PlayGTA5.exe",
    L"C:\\Grand Theft Auto V Lite\\Download\\PlayGTA.exe"
  };
  for(const auto& p:candidates) if(IsFile(p)) return p;
  return L"";
}

void OpenGameFolder(){
  if(!EnsureGameDirectory()){ MessageBoxW(GetActiveWindow(),L"Não foi possível criar C:\\Grand Theft Auto V Lite\\Download. Verifique as permissões do container.",L"Erro de diretório",MB_ICONERROR); return; }
  ShellExecuteW(nullptr,L"open",GameDirectory().c_str(),nullptr,nullptr,SW_SHOWNORMAL);
  SetStatus(L"Pasta aberta: "+GameDirectory());
}

void LaunchDetectedGame(){
  if(!EnsureGameDirectory()){ MessageBoxW(GetActiveWindow(),L"Não foi possível preparar C:\\Grand Theft Auto V Lite\\Download.",L"Erro de diretório",MB_ICONERROR); return; }
  std::wstring file=FindPlayGTA();
  if(file.empty()){
    std::wstring first=FindFirstPart();
    if(!first.empty()){
      std::wstring missing;
      int lastPart=first.rfind(L".000")!=std::wstring::npos?6:6;
      if(!AllPartsPresent(first,lastPart,missing)){
        std::wstring msg=L"PlayGTA5.exe/PlayGTA.exe não foi encontrado e falta a parte:\n"+missing+L"\n\nColoque todas as partes numeradas na mesma pasta:\n"+GameDirectory();
        MessageBoxW(GetActiveWindow(),msg.c_str(),L"Partes incompletas",MB_OK|MB_ICONWARNING); SetStatus(L"Partes incompletas."); return;
      }
      if(!ExtractLocalArchive(first)) return;
      file=FindPlayGTA();
    }
    if(file.empty()){
      std::wstring msg=L"PlayGTA5.exe/PlayGTA.exe não foi encontrado.\n\nColoque o executável ou as partes .001 até .006 em:\n"+GameDirectory()+L"\n\nou use o botão PlayGTA.exe para escolher outro local.";
      MessageBoxW(GetActiveWindow(),msg.c_str(),L"Arquivo não encontrado",MB_OK|MB_ICONWARNING);
      SetStatus(L"PlayGTA.exe não encontrado nos caminhos configurados."); return;
    }
  }
  HINSTANCE result=ShellExecuteW(nullptr,L"open",file.c_str(),nullptr,GameDirectory().c_str(),SW_SHOWNORMAL);
  if((INT_PTR)result<=32) MessageBoxW(GetActiveWindow(),L"O arquivo foi encontrado, mas não pôde ser iniciado neste ambiente.",L"Erro ao iniciar",MB_ICONERROR);
  else SetStatus(L"PlayGTA.exe iniciado: "+file);
}

void OpenLocalProgram(){
  std::wstring file=PickFile(); if(file.empty()) return;
  HINSTANCE result=ShellExecuteW(nullptr,L"open",file.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
  if((INT_PTR)result<=32){ MessageBoxW(GetActiveWindow(),L"Não foi possível abrir o arquivo escolhido.",L"Erro ao iniciar",MB_ICONERROR); return; }
  SetStatus(L"Arquivo local iniciado: "+file);
}

void OpenSupportLink(int id){
  if(id==ID_YOUTUBE){ ShellExecuteW(nullptr,L"open",L"https://youtube.com/@arthur2345-thydf",nullptr,nullptr,SW_SHOWNORMAL); SetStatus(L"Abrindo o canal YouTube de ArthurJALDev_V…"); return; }
  const wchar_t username[]=L"arthur_.12345678";
  if(OpenClipboard(GetActiveWindow())){ EmptyClipboard(); HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,(wcslen(username)+1)*sizeof(wchar_t)); if(mem){ void* p=GlobalLock(mem); memcpy(p,username,(wcslen(username)+1)*sizeof(wchar_t)); GlobalUnlock(mem); SetClipboardData(CF_UNICODETEXT,mem); } CloseClipboard(); }
  ShellExecuteW(nullptr,L"open",L"https://discord.gg/VCE3s78bB",nullptr,nullptr,SW_SHOWNORMAL); SetStatus(L"Convite do Discord aberto: discord.gg/VCE3s78bB");
}

void PaintRounded(HDC dc, RECT r, COLORREF color, int radius){ HBRUSH b=CreateSolidBrush(color); HPEN p=CreatePen(PS_SOLID,1,color); auto ob=SelectObject(dc,b); auto op=SelectObject(dc,p); RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius); SelectObject(dc,ob); SelectObject(dc,op); DeleteObject(b); DeleteObject(p); }
void Text(HDC dc,const wchar_t* t,int x,int y,int size,COLORREF color,bool bold=false){ HFONT f=CreateFontW(size,0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI"); auto old=SelectObject(dc,f); SetTextColor(dc,color); SetBkMode(dc,TRANSPARENT); TextOutW(dc,x,y,t,(int)wcslen(t)); SelectObject(dc,old); DeleteObject(f); }
void ChromeText(HDC dc,const wchar_t* t,int x,int y,int size){
  // Camadas de sombra, contorno e brilho para um acabamento cromado 3D.
  Text(dc,t,x+4,y+5,size,RGB(18,7,18),true); Text(dc,t,x+2,y+2,size,RGB(99,44,83),true);
  Text(dc,t,x,y,size,RGB(255,255,255),true); Text(dc,t,x,y-1,size,RGB(226,226,234),true);
}
void Gradient(HDC dc, RECT r, COLORREF top, COLORREF bottom){
  TRIVERTEX v[2]{}; v[0].x=r.left; v[0].y=r.top; v[0].Red=GetRValue(top)<<8; v[0].Green=GetGValue(top)<<8; v[0].Blue=GetBValue(top)<<8;
  v[1].x=r.right; v[1].y=r.bottom; v[1].Red=GetRValue(bottom)<<8; v[1].Green=GetGValue(bottom)<<8; v[1].Blue=GetBValue(bottom)<<8; GRADIENT_RECT gr{0,1}; GradientFill(dc,v,2,&gr,1,GRADIENT_FILL_RECT_V);
}
void MetallicBorder(HDC dc, RECT r){
  HPEN outer=CreatePen(PS_SOLID,5,RGB(226,226,234)); HGDIOBJ old=SelectObject(dc,outer); SelectObject(dc,GetStockObject(HOLLOW_BRUSH)); RoundRect(dc,r.left,r.top,r.right,r.bottom,20,20); SelectObject(dc,old); DeleteObject(outer);
  RECT inner{r.left+6,r.top+6,r.right-6,r.bottom-6}; HPEN pink=CreatePen(PS_SOLID,2,RGB(255,118,190)); old=SelectObject(dc,pink); RoundRect(dc,inner.left,inner.top,inner.right,inner.bottom,16,16); SelectObject(dc,old); DeleteObject(pink);
}

LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM w, LPARAM l){
  switch(msg){
    case WM_CREATE:{
      gBanner=(HBITMAP)LoadImageW(gInstance,MAKEINTRESOURCEW(IDB_BANNER),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
      gLogo=(HBITMAP)LoadImageW(gInstance,MAKEINTRESOURCEW(IDB_LOGO),IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
      gStatus=CreateWindowW(L"STATIC",L"Pronto para começar.",WS_CHILD|WS_VISIBLE,34,585,860,28,h,nullptr,gInstance,nullptr);
      gInfo=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP,34,370,860,200,h,nullptr,gInstance,nullptr); SetWindowTextW(gInfo,EnvironmentReport().c_str());
      SendMessageW(gInfo,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
      // Atualização rápida de 100 ms; timers de 1 ms não são confiáveis no Winlator/Wine.
      SetTimer(h,1,100,nullptr);
      CreateWindowW(L"BUTTON",L"Criar container",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,34,300,180,46,h,(HMENU)ID_VALIDATE,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"Abrir Grand Theft Auto V Lite",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,228,300,245,46,h,(HMENU)ID_PLAY,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"PlayGTA.exe",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,487,300,150,46,h,(HMENU)ID_OPEN,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"Sair",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,34,620,90,34,h,(HMENU)ID_EXIT,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"Abrir pasta do jogo",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,134,620,190,34,h,(HMENU)ID_FOLDER,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"YouTube",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,334,620,140,34,h,(HMENU)ID_YOUTUBE,gInstance,nullptr);
      CreateWindowW(L"BUTTON",L"Discord",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,484,620,140,34,h,(HMENU)ID_DISCORD,gInstance,nullptr);
      return 0; }
    case WM_COMMAND:
      if(LOWORD(w)==ID_EXIT) DestroyWindow(h);
      else if(LOWORD(w)==ID_VALIDATE){ SetWindowTextW(gInfo,EnvironmentReport().c_str()); SetStatus(L"Ambiente validado. Container pronto para arquivos locais."); InvalidateRect(h,nullptr,TRUE); }
      else if(LOWORD(w)==ID_PLAY) LaunchDetectedGame();
      else if(LOWORD(w)==ID_OPEN) OpenLocalProgram();
      else if(LOWORD(w)==ID_FOLDER) OpenGameFolder();
      else if(LOWORD(w)==ID_YOUTUBE || LOWORD(w)==ID_DISCORD) OpenSupportLink(LOWORD(w));
      return 0;
    case WM_TIMER:
      if(w==1 && gInfo){ int firstLine=(int)SendMessageW(gInfo,EM_GETFIRSTVISIBLELINE,0,0); SetWindowTextW(gInfo,EnvironmentReport().c_str()); SendMessageW(gInfo,EM_LINESCROLL,0,firstLine); InvalidateRect(h,nullptr,FALSE); UpdateWindow(h); }
      return 0;
    case WM_DRAWITEM:{
      auto d=(DRAWITEMSTRUCT*)l; bool primary=d->CtlID==ID_VALIDATE; COLORREF c=primary?PINK:RGB(58,32,53); PaintRounded(d->hDC,d->rcItem,c,9); const wchar_t* label=d->CtlID==ID_VALIDATE?L"Criar container":d->CtlID==ID_PLAY?L"Abrir Grand Theft Auto V Lite":d->CtlID==ID_OPEN?L"PlayGTA.exe":d->CtlID==ID_FOLDER?L"Abrir pasta do jogo":d->CtlID==ID_YOUTUBE?L"YouTube":d->CtlID==ID_DISCORD?L"Discord":L"Sair"; SetTextColor(d->hDC,WHITE); SetBkMode(d->hDC,TRANSPARENT); DrawTextW(d->hDC,label,-1,&d->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE); return TRUE; }
    case WM_PAINT:{ PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps); RECT r; GetClientRect(h,&r); Gradient(dc,r,RGB(20,10,25),RGB(71,24,64));
      RECT card{22,20,r.right-22,270}; PaintRounded(dc,card,PANEL,20);
      if(gBanner){ HDC mem=CreateCompatibleDC(dc); HGDIOBJ old=SelectObject(mem,gBanner); StretchBlt(dc,card.left+3,card.top+3,card.right-card.left-6,card.bottom-card.top-6,mem,0,0,820,450,SRCCOPY); SelectObject(mem,old); DeleteDC(mem); HBRUSH shade=CreateSolidBrush(RGB(19,13,22)); RECT overlay{card.left+3,card.top+3,card.left+390,card.bottom-3}; FillRect(dc,&overlay,shade); DeleteObject(shade); }
      MetallicBorder(dc,card);
      ChromeText(dc,L"ArthurJALDev_V",44,43,14); ChromeText(dc,L"GTA 5 FIVE LITE",44,70,30); Text(dc,L"Launcher rosa cromado • ambiente 3D",44,118,21,RGB(255,157,211),true); Text(dc,L"Valide o ambiente e abra apenas programas escolhidos localmente.",44,160,13,MUTED,false); Text(dc,L"MODO PAISAGEM  •  ARQUIVOS LOCAIS",44,215,10,RGB(220,178,211),true);
      HPEN shine=CreatePen(PS_SOLID,2,RGB(255,214,238)); HGDIOBJ oldPen=SelectObject(dc,shine); MoveToEx(dc,42,236,nullptr); LineTo(dc,430,236); SelectObject(dc,oldPen); DeleteObject(shine);
      RECT logoCard{495,70,730,260}; PaintRounded(dc,logoCard,RGB(224,64,160),18); MetallicBorder(dc,logoCard);
      if(gLogo){ HDC logoDC=CreateCompatibleDC(dc); HGDIOBJ logoOld=SelectObject(logoDC,gLogo); StretchBlt(dc,510,78,205,158,logoDC,0,0,260,200,SRCCOPY); SelectObject(logoDC,logoOld); DeleteDC(logoDC); }
      // O cartão exibe somente a imagem do emblema; a assinatura permanece no rodapé.
      Text(dc,L"DIAGNÓSTICO DO AMBIENTE",34,350,10,RGB(255,138,199),true); ChromeText(dc,L"ArthurJALDev_V",755,680,12); EndPaint(h,&ps); return 0; }
    case WM_DESTROY: KillTimer(h,1); if(gBanner) DeleteObject(gBanner); if(gLogo) DeleteObject(gLogo); PostQuitMessage(0); return 0;
  } return DefWindowProcW(h,msg,w,l);
}
int WINAPI wWinMain(HINSTANCE h,HINSTANCE, PWSTR,int show){ gInstance=h; WNDCLASSW wc{}; wc.hInstance=h; wc.lpfnWndProc=WndProc; wc.lpszClassName=L"ArthurJALDev_V"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hIcon=LoadIconW(h,MAKEINTRESOURCEW(IDI_CUSTOM)); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); RegisterClassW(&wc); HWND win=CreateWindowExW(0,wc.lpszClassName,L"Grand_Theft_Auto_V_Lite",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,140,70,950,730,nullptr,nullptr,h,nullptr); ShowWindow(win,show); UpdateWindow(win); MSG m; while(GetMessageW(&m,nullptr,0,0)){TranslateMessage(&m);DispatchMessageW(&m);} return (int)m.wParam; }

// Protótipo seguro: não inclui, baixa ou distribui jogos, APKs ou arquivos de terceiros.
