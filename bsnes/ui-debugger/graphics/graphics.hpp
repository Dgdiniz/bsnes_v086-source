
struct GraphicsConverter : Window {
  VerticalLayout layout;
    HorizontalLayout controlLayout;
      Label dataLabel;
      LineEdit dataAddress;
      Label tilemapLabel;
      LineEdit tilemapAddress;
      Button saveGraphics;
      ComboBox modeSelection;
      Widget spacer;
      Button compressImage;
    Canvas canvas;

    void saveGraphicsData();
    void compressGraphics();

    GraphicsConverter();
};

extern GraphicsConverter *graphicsConverter;
