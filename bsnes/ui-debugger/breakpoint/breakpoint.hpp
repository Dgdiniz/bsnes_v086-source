struct BreakpointEntry : HorizontalLayout {
  CheckBox enable;
  LineEdit addr;
  LineEdit data;
  ComboBox type;
  ComboBox source;

  BreakpointEntry();
};

#define BRK_NUMBER 15

struct BreakpointEditor : Window {
  VerticalLayout layout;
  BreakpointEntry breakpointEntry[BRK_NUMBER];

  struct Breakpoint {
    enum : unsigned { Read, Write, Exec };
    enum : unsigned { CPU, SMP, VRAM, OAM, CGRAM };
    unsigned id;
    bool compare;
    unsigned addr;
    unsigned data;
    unsigned type;
    unsigned source;
  };
  vector<Breakpoint> breakpoint;
  vector<Breakpoint> breakpointReadCPU;
  vector<Breakpoint> breakpointWriteCPU;
  vector<Breakpoint> breakpointExecCPU;
  vector<Breakpoint> breakpointReadSMP;
  vector<Breakpoint> breakpointWriteSMP;
  vector<Breakpoint> breakpointExecSMP;
  vector<Breakpoint> breakpointReadVRAM;
  vector<Breakpoint> breakpointWriteVRAM;
  vector<Breakpoint> breakpointReadOAM;
  vector<Breakpoint> breakpointWriteOAM;
  vector<Breakpoint> breakpointReadCGRAM;
  vector<Breakpoint> breakpointWriteCGRAM;
  void synchronize();

  bool save(const string path){
	  file fp;
	  if(fp.open(path, file::mode::write)) {
		  for(unsigned i = 0; i < BRK_NUMBER; i++) {
			  string output;
			  if (breakpointEntry[i].enable.checked())
				  output.append("enabled,");
			  else
				  output.append("disabled,");

			  output.append(breakpointEntry[i].addr.text(), ",");

			  output.append(breakpointEntry[i].data.text(), ",");

			  output.append(breakpointEntry[i].type.selection(), ",");

			  output.append(breakpointEntry[i].source.selection());

			  output.append("\r\n");
			  fp.print(output);
		  }

		  fp.close();
		  return true;
	  } else {
		  return false;
	  }
  }

  bool load(const string path) {
	  string data;
	  if(data.readfile(path) == true) {
		  data.replace("\r", "");
		  lstring line;
		  line.split("\n", data);

		  for(unsigned i = 0; i < line.size()-1; i++) {
			  lstring fields;
			  fields.split(",", line[i]);

			  fields[0].trim();
			  if (fields[0] == "enabled") {
				  breakpointEntry[i].enable.setChecked(true);
				  breakpointEntry[i].addr.setEnabled(false);
				  breakpointEntry[i].data.setEnabled(false);
				  breakpointEntry[i].type.setEnabled(false);
				  breakpointEntry[i].source.setEnabled(false);
			  }

			  fields[1].trim();
			  breakpointEntry[i].addr.setText({fields[1]});

			  fields[2].trim();
			  breakpointEntry[i].data.setText({fields[2]});

			  fields[3].trim();
			  breakpointEntry[i].type.setSelection(atoi( fields[3] ));

			  fields[4].trim();
			  breakpointEntry[i].source.setSelection(atoi( fields[4] ));
		  }

		  return true;
	  } else {
		  return false;
	  }
  }


  bool testReadCPU(uint24 addr);
  bool testWriteCPU(uint24 addr, uint8 data);
  bool testExecCPU(uint24 addr);

  bool testReadSMP(uint16 addr);
  bool testWriteSMP(uint16 addr, uint8 data);
  bool testExecSMP(uint16 addr);

  bool testReadVRAM(uint16 addr);
  bool testWriteVRAM(uint16 addr, uint8 data);

  bool testReadOAM(uint16 addr);
  bool testWriteOAM(uint16 addr, uint8 data);

  bool testReadCGRAM(uint16 addr);
  bool testWriteCGRAM(uint16 addr, uint8 data);

  BreakpointEditor();
};


extern BreakpointEditor *breakpointEditor;
