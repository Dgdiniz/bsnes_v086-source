struct VRAMViewer : Window {
  VerticalLayout layout;
    HorizontalLayout controlLayout;
      Label modeLabel;
      ComboBox modeSelection;
      LineEdit addr;
      Button cleantile;
      Button dump;
      Widget spacer;
      CheckBox autoUpdate;
      Button update;
    Canvas canvas;

  void updateTiles();
  void cleanTile();
  void dumpVram();
  VRAMViewer();
};

extern VRAMViewer *vramViewer;
