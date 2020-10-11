
#include "../base.hpp"
GraphicsConverter *graphicsConverter = nullptr;

GraphicsConverter::GraphicsConverter() {
	setTitle("Graphics Editor");

	dataLabel.setText("Data:");
	tilemapLabel.setText("Tilemap:");

	setGeometry({128, 128, 512, 480});
	canvas.setSize({512, 480});

	compressImage.setText("Compress");
	saveGraphics.setText("Save Graphics");

	modeSelection.append("2bpp", "4bpp", "8bpp");

	layout.setMargin(5);
	layout.append(controlLayout, {~0, 0}, 5);
	controlLayout.append(dataLabel, {0, 0}, 5);
	controlLayout.append(dataAddress, {50, 0}, 5);
	controlLayout.append(tilemapLabel, {0, 0}, 5);
	controlLayout.append(tilemapAddress, {50, 0}, 5);
	controlLayout.append(saveGraphics, {80, 0}, 5);
	controlLayout.append(modeSelection, {0, 0}, 5);
	controlLayout.append(spacer, {~0, 0});
	controlLayout.append(compressImage, {80, 0});
	layout.append(canvas, {~0, ~0});
	append(layout);

	saveGraphics.onActivate = { &GraphicsConverter::saveGraphicsData, this };
	compressImage.onActivate = { &GraphicsConverter::compressGraphics, this };

	windowManager->append(this, "GraphicsConverter");
}

void GraphicsConverter::saveGraphicsData() {
	string filename_vram  = { interface->pathName, "graphics/vram.bin" };
	string filename_oam   = { interface->pathName, "graphics/oam.bin" };
	string filename_cgram = { interface->pathName, "graphics/cgram.bin" };
	string filename_tpl   = { interface->pathName, "graphics/cgram.tpl" };
	string filename_tm    = { interface->pathName, "graphics/cgram_tilemolester.tpl" };
	string filename_image = { interface->pathName, "graphics/image.smc" };

	unsigned dataAddr = hex(dataAddress.text());
	unsigned tilemapAddr = hex(tilemapAddress.text());
	unsigned mode = modeSelection.selection();

	file fp_vram;
	file fp_oam;
	file fp_cgram;
	file fp_cgram_tpl;
	file fp_cgram_tm;

	file fp_image;

	if(fp_vram.open(filename_vram, file::mode::write) == false) return;
	for(unsigned addr = 0; addr <= 0xffff; addr++) fp_vram.write(SNES::ppu.vram[addr]);

	if(fp_oam.open(filename_oam, file::mode::write) == false) return;
	for(unsigned addr = 0; addr <= 0x021f; addr++) fp_oam.write(SNES::ppu.oam[addr]);

	if(fp_cgram.open(filename_cgram, file::mode::write) == false) return;
	for(unsigned addr = 0; addr <= 0x01ff; addr++) fp_cgram.write(SNES::ppu.cgram[addr]);

	if(fp_cgram_tpl.open(filename_tpl, file::mode::write) == false) return;
	fp_cgram_tpl.write(0x54);
	fp_cgram_tpl.write(0x50);
	fp_cgram_tpl.write(0x4c);
	fp_cgram_tpl.write(0x2);
	for(unsigned addr = 0; addr <= 0x01ff; addr++) fp_cgram_tpl.write(SNES::ppu.cgram[addr]);

	if(fp_cgram_tm.open(filename_tm, file::mode::write) == false) return;
	fp_cgram_tm.write(0x54);
	fp_cgram_tm.write(0x50);
	fp_cgram_tm.write(0x4c);
	fp_cgram_tm.write(0x2);
	for(unsigned addr = 0; addr <= 0x01ff; addr++) fp_cgram_tm.write(SNES::ppu.cgram[addr]);

	uint8_t current=0;
	bool high = false;
	if(fp_image.open(filename_image, file::mode::write) == false) return;
	for(unsigned addr = tilemapAddr; addr < tilemapAddr + 0x800; addr+=2) {
		uint16_t tilemap = (SNES::ppu.vram[addr] & 0xff) | ((SNES::ppu.vram[addr+1] & 0xff) << 8);
		uint16_t tile_position;
		unsigned tileSize;

		if (mode == 1) {
			tile_position = 8*4*(tilemap & 0x3ff);
			tileSize = 32;
		}
		else if (mode == 0) {
			tile_position = 8*2*(tilemap & 0x3ff);
			tileSize = 16;
		}

		for(unsigned i = dataAddr+tile_position; i<dataAddr+tile_position+tileSize; i++) {
			fp_image.write(SNES::ppu.vram[i]);
		}

	}

	debugger->print("Graphics exported to folder /graphics\n");
}

struct graphicsTile {
	uint8_t data[32];

//	graphicsTile() {
//		for(unsigned i = 0; i< 32; i++) data[i] = 0;
//	}
	inline bool operator  == (const graphicsTile &tile) {
		for(unsigned i = 0; i < 32; i++) {
			if (data[i] != tile.data[i]) return false;
		}
		return true;
	}
};

void GraphicsConverter::compressGraphics() {
	struct graphicsTile tiles[32*28];
	struct graphicsTile compressed[32*28];
	unsigned numTiles=0;
	string filename_image = { interface->pathName, "graphics/image.smc" };
	string filename_image_compressed = { interface->pathName, "graphics/image_compressed.smc" };
	file fp_image;
	file fp_image_compressed;

	graphicsTile tile_horizontal;
	graphicsTile tile_vertical;
	graphicsTile tile_vertical_horizontal;

	uint8_t tilemap[0x800];
	file fp_tilemap;
	string filename_tilemap = { interface->pathName, "graphics/tilemap.bin" };
	file fp_tilemap_original;
	string filename_tilemap_original = { interface->pathName, "graphics/tilemap_original.bin" };

	if(fp_image.open(filename_image, file::mode::read) == false) return;

	for(unsigned i = 0; i< 32*28; i++) { //read tiles
		fp_image.read(tiles[i].data, 32);
	}

	for(unsigned i = 0; i< 32*28*2; i++) {//read tilemap
		tilemap[i] = SNES::ppu.vram[0xe000 + i];
	}

	if(fp_tilemap_original.open(filename_tilemap_original, file::mode::write) == false) return;
	for(unsigned i = 0; i < 32*28*2; i++) {
		fp_tilemap_original.write(tilemap[i]);
	}

	compressed[0] = tiles[0];
	numTiles++;

	bool found = false;

	for(unsigned i = 1; i< 32*28; i++) {
		found = false;

		//monta o tile horizontal
		for(unsigned x = 0; x < 8; x++) {
			for(unsigned y = 0; y < 4; y++) {
				tile_horizontal.data[8*x + y] = ((tiles[i].data[8*x + 4-y] >> 4) & 0xf) | ((tiles[i].data[8*x + 4-y] << 4) & 0xf0);
			}
		}

		//monta o tile vertical
		for(unsigned x = 0; x < 8; x++) {
			for(unsigned y = 0; y < 4; y++) {
				tile_vertical.data[x + 8*y] = tiles[i].data[8*(7-x)+y];
			}
		}

		//monta o tile vertical horizontal
		for(unsigned x = 0; x < 8; x++) {
			for(unsigned y = 0; y < 4; y++) {
				tile_vertical_horizontal.data[8*x + y] =((tile_vertical.data[8*x + 4-y] >> 4) & 0xf) | ((tile_vertical.data[8*x + 4-y] << 4) & 0xf0);
			}
		}


		for(unsigned j = 0; j < numTiles; j++) {
			if (tiles[i] == compressed[j]) {
				found = true;
				tilemap[2*i+1] = (tilemap[2*i+1] & 0xfc) | ((j >> 8) & 0x3);
				tilemap[2*i] = j & 0xff;
				break;
			}

			if ((found == false) && (tile_horizontal == compressed[j])) {
				found = true;
				tilemap[2*i+1] = (tilemap[2*i+1] & 0xfc) | ((j >> 8) & 0x3);
				tilemap[2*i] = j & 0xff;
				tilemap[2*i+1] |= 0x40; //horizontal bit
				break;
			}

			if ((found == false) && (tile_vertical == compressed[j])) {
				found = true;
				tilemap[2*i+1] = (tilemap[2*i+1] & 0xfc) | ((j >> 8) & 0x3);
				tilemap[2*i] = j & 0xff;
				tilemap[2*i+1] |= 0x80; //vertical bit
				break;
			}

			if ((found == false) && (tile_vertical_horizontal == compressed[j])) {
				found = true;
				tilemap[2*i+1] = (tilemap[2*i+1] & 0xfc) | ((j >> 8) & 0x3);
				tilemap[2*i] = j & 0xff;
				tilemap[2*i+1] |= 0x40; //horizontal bit
				tilemap[2*i+1] |= 0x80; //vertical bit
				break;
			}
		}

		if (found == false) {
			compressed[numTiles] = tiles[i];
			tilemap[2*i+1] = (tilemap[2*i+1] & 0xfc) | ((numTiles >> 8) & 0x3);
			tilemap[2*i] = numTiles & 0xff;
			numTiles++;
		}
	}

	if(fp_image_compressed.open(filename_image_compressed, file::mode::write) == false) return;
	for(unsigned i = 0; i <= numTiles; i++) {
		for(unsigned j = 0; j < 32; j++)
			fp_image_compressed.write(compressed[i].data[j]);
	}

	if(fp_tilemap.open(filename_tilemap, file::mode::write) == false) return;
	for(unsigned i = 0; i < 32*28*2; i++) {
		fp_tilemap.write(tilemap[i]);
	}

	debugger->print("Graphics compressed.\n");
}
