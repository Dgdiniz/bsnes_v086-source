#include "base.hpp"

Application *application = nullptr;

Application::Application(int argc, char **argv) {
  application = this;
  quit = false;
  bool outOfFocus = false;

  {
    char path[PATH_MAX];
    auto unused = ::realpath(argv[0], path);
    basepath = dir(path);
    unused = ::userpath(path);
    userpath = path;
    if(Intrinsics::platform() == Intrinsics::Platform::Windows) {
      userpath.append("laevateinn/");
    } else {
      userpath.append(".config/laevateinn/");
    }
    mkdir(userpath, 0755);
  }

  if(Intrinsics::platform() == Intrinsics::Platform::Windows) {
    proportionalFont = "Tahoma, 8";
    proportionalFontBold = "Tahoma, 8, Bold";
    monospaceFont = "Lucida Console, 8";
  } else {
    proportionalFont = "Sans, 8";
    proportionalFontBold = "Sans, 8, Bold";
    monospaceFont = "Liberation Mono, 8";
  }

  settings = new Settings;
  settings->load();

  string foldername;
  if(argc >= 2) foldername = argv[1];
//if(!directory::exists(foldername)) foldername = "/media/sdb1/root/cartridges/The Legend of Zelda - A Link to the Past (US).sfc/";
  if(!directory::exists(foldername)) foldername = DialogWindow::folderSelect(Window::None, settings->folderpath);
  if(!foldername.endswith(".sfc/")) return;
  lstring contents = directory::files(foldername, "*.sfc");
  if(contents.size() != 1) return;
  string filename = { foldername, contents[0] };
  if(!file::exists(filename)) return;

  //save path for later; remove cartridge name from path
  settings->folderpath = foldername;
  settings->folderpath.rtrim<1>("/");
  settings->folderpath = dir(settings->folderpath);

  interface = new Interface;
  debugger = new Debugger;
  tracer = new Tracer;
  windowManager = new WindowManager;
  consoleWindow = new ConsoleWindow;
  aboutWindow = new AboutWindow;
  videoWindow = new VideoWindow;
  cpuDebugger = new CPUDebugger;
  cpuRegisterEditor = new CPURegisterEditor;
  smpDebugger = new SMPDebugger;
  smpRegisterEditor = new SMPRegisterEditor;
  memoryEditor = new MemoryEditor;
  breakpointEditor = new BreakpointEditor;
  propertiesViewer = new PropertiesViewer;
  vramViewer = new VRAMViewer;
  graphicsConverter = new GraphicsConverter;

  windowManager->loadGeometry();
  consoleWindow->setVisible();
  videoWindow->setVisible();
  cpuDebugger->setVisible();
  propertiesViewer->setVisible();
  vramViewer->setVisible();
  memoryEditor->updateView();
  memoryEditor->setVisible();
  breakpointEditor->setVisible();
  graphicsConverter->setVisible();
  consoleWindow->setFocused();

  if(audio.init() == false) {
    audio.driver("None");
    audio.init();
  }
  audio.set(Audio::Synchronize, settings->synchronizeAudio);
  audio.set(Audio::Frequency, 32000u);

  if(interface->loadCartridge(filename) == false) return;
  cpuDebugger->updateDisassembly();
  smpDebugger->updateDisassembly();
  memoryEditor->selectSource();
  propertiesViewer->updateProperties();
  vramViewer->updateTiles();

  breakpointEditor->load({ interface->pathName, "debug/breakpoints.brk" });
  breakpointEditor->synchronize();

  while(quit == false) {
    OS::processEvents();
    debugger->run();

    if (outOfFocus == false) {
    	if ((!consoleWindow->focused()) && (!videoWindow->focused()) && (!memoryEditor->focused()) && (!breakpointEditor->focused()) && (!cpuDebugger->focused()) && (!propertiesViewer->focused()) && (!vramViewer->focused())&& (!graphicsConverter->focused())) {
    		outOfFocus = true;
    	}
    }
    else {
    	if ((consoleWindow->focused()) || (videoWindow->focused()) || (memoryEditor->focused()) || (breakpointEditor->focused()) || (cpuDebugger->focused()) || (propertiesViewer->focused()) || (vramViewer->focused()) || (graphicsConverter->focused())) {
    		outOfFocus = false;
    		consoleWindow->setFocused();
    		memoryEditor->setFocused();
    		breakpointEditor->setFocused();
    		cpuDebugger->setFocused();
    		propertiesViewer->setFocused();
    		vramViewer->setFocused();
    		videoWindow->setFocused();
    		graphicsConverter->setFocused();
    	}
    }
  }

  interface->saveMemory();
  windowManager->saveGeometry();
  settings->save();
  breakpointEditor->save({ interface->pathName, "debug/breakpoints.brk" });

//  string filenameBrkPoints;
//  unsigned char teste[10];
//  teste[0] = 1;
//
//  filenameBrkPoints = { interface->pathName, "debug/breakpoints.brk" };
//  file::write(filenameBrkPoints, teste, 10);
}

Application::~Application() {
  delete vramViewer;
  delete propertiesViewer;
  delete breakpointEditor;
  delete memoryEditor;
  delete smpRegisterEditor;
  delete smpDebugger;
  delete cpuRegisterEditor;
  delete cpuDebugger;
  delete videoWindow;
  delete aboutWindow;
  delete consoleWindow;
  delete windowManager;
  delete tracer;
  delete debugger;
  delete interface;
  delete settings;
}

int main(int argc, char **argv) {
  new Application(argc, argv);
  delete application;
  return 0;
}
