#include <fern.h>
#include <fern_common.h>
#include <SDL2/SDL.h>

namespace fern {
	CRenderer::CRenderer() 
	: m_screen(160,144) {
		m_window = nullptr;
	}
	CRenderer::~CRenderer() {
		
	}

	auto CRenderer::window_create() -> void {
		m_window = SDL_CreateWindow("fern",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			160,144,
			0
		);
		
		m_renderer = SDL_CreateRenderer(
			m_window,-1,
			SDL_RENDERER_SOFTWARE
				| SDL_RENDERER_PRESENTVSYNC
		);
	}

	auto CRenderer::present() -> void {
		SDL_RenderClear(m_renderer);
		for(int y=0; y<m_screen.height(); y++) {
			for(int x=0; x<m_screen.width(); x++) {
				const auto color = m_screen.dot_access(x,y);
				SDL_SetRenderDrawColor(m_renderer,color.r,color.g,color.b,255);
				SDL_RenderDrawPoint(m_renderer,x,y);
			}
		}
		SDL_RenderPresent(m_renderer);
	}
	auto CRenderer::draw_line(int draw_y) -> void {
		if(draw_y < 0 || draw_y >= 144) return;
		/*const fern::CColor palet_gray[4] = {
			fern::CColor(255,255,255),
			fern::CColor(192,192,192),
			fern::CColor(112,112,112),
			fern::CColor(12,12,12)
		};*/
		const std::array<fern::CColor,4> palet_gray = {
			fern::CColor(0xff,0xf6,0xd3),
			fern::CColor(0xf9,0xa8,0x75),
			fern::CColor(0xeb,0x6b,0x6f),
			fern::CColor(0x7c,0x3f,0x58)
		};

		auto& mem = emu()->mem;
		const int lcdc = mem.m_io.m_LCDC;
		std::array<std::array<int,4>,2> obp_table;
		std::array<int,4> bgp_table;

		// create bgp lookup table
		int bgp_shifter = mem.m_io.m_BGP;
		std::array<int,2> obp_shifter = { mem.m_io.m_OBP[0], mem.m_io.m_OBP[1] };
		for(int i=0; i<4; i++) {
			obp_table[0][i] = obp_shifter.at(0) & 3;
			obp_table[1][i] = obp_shifter.at(1) & 3;
			bgp_table.at(i) = bgp_shifter & 3;
			obp_shifter.at(0) >>= 2;
			obp_shifter.at(1) >>= 2;
			bgp_shifter >>= 2;
		}

		// render to screen
		const int bgscroll_x = mem.m_io.m_SCX;
		const int bgscroll_y = mem.m_io.m_SCY;
		
		const int fetch_y = (bgscroll_y + draw_y) & 0xFF;
		size_t addr_chrbase = 0x1800;
		size_t addr_mapbase = 0x1800;
		if(lcdc & RFlagLCDC::chr8000) addr_chrbase -= 0x0800;
		if(lcdc & RFlagLCDC::map9C00) addr_mapbase += 0x0400;
		const size_t addr_mapline = addr_mapbase + ((fetch_y/8) * 0x20);

		for(int draw_x=0; draw_x<160; draw_x++) {
			int fetch_x = (bgscroll_x + draw_x) & 0xFF;
			// fetch tile
			int tile = static_cast<int8_t>(mem.m_vram.at(addr_mapline + (fetch_x/8)));
			tile -= 0x80;
			int tileaddr = addr_chrbase + tile * 0x10;
			// get pixel
			tileaddr += (draw_y&7)*2;
			int lineA = mem.m_vram[tileaddr];
			int lineB = mem.m_vram[tileaddr+1];
			int dotA = (lineA >> (7-(fetch_x&7))) & 1;
			int dotB = (lineB >> (7-(fetch_x&7))) & 1;
			int dot = dotA | (dotB<<1);
			m_screen.dot_set(draw_x,draw_y,palet_gray[bgp_table[dot]]);
		//	m_screen.dot_set(draw_x,draw_y,palet_gray[dot]);
		}

		// draw sprites ---------------------------------@/
		const bool spr_size2x = (mem.m_io.m_LCDC & 0x04) != 0;
		const int spr_height = spr_size2x ? 16 : 8;
		
		for(int spr_idx=0; spr_idx<40; spr_idx++) {
			const std::array<int,4> oamdata = { 
				mem.m_oam[spr_idx*4 + 0],
				mem.m_oam[spr_idx*4 + 1],
				mem.m_oam[spr_idx*4 + 2],
				mem.m_oam[spr_idx*4 + 3]
			};

			const int oamdat_y = oamdata[0] - 16;
			const int oamdat_x = oamdata[1] - 8;
			const int oamdat_palet = (oamdata[3] >> 4) & 1;
			const bool flipX = (oamdata[3]>>5) & 1;
			const bool flipY = (oamdata[3]>>6) & 1;

			auto& cur_paltable = obp_table.at(oamdat_palet);
			if(oamdat_y <= -16 || oamdat_y >= 144) continue;
			if(oamdat_x <= -8 || oamdat_x >= 160) continue;
			if(draw_y < oamdat_y) continue;
			if((draw_y - oamdat_y) >= spr_height) continue;

			// get relative line of the sprite to draw
			int line_y = (draw_y - oamdat_y);
			if(flipY) line_y = (spr_height-1) - line_y;

			// get tile address
			int tileaddr = 0x0000 + (oamdata[2] * 0x10);
			tileaddr += line_y * 2;
			if(spr_size2x) tileaddr &= 0xFFFE;
			int lineA = mem.m_vram[tileaddr];
			int lineB = mem.m_vram[tileaddr+1];

			for(int ix=0; ix<8; ix++) {
				int x = ix;
				if(oamdat_x+x < 0) continue;
				if(oamdat_x+x >= 160) continue;

				int dotA = (lineA >> (7-x)) & 1;
				int dotB = (lineB >> (7-x)) & 1;
				int dot = dotA | (dotB<<1);
				if(dot == 0) continue;
				if(flipX) x = (7-x);
				m_screen.dot_access(oamdat_x+x,draw_y) = 
					palet_gray.at(cur_paltable.at(dot));
			}
		}
	}

	// screen -------------------------------------------@/
	CScreen::CScreen(int new_width, int new_height) {
		m_width = new_width;
		m_height = new_height;

		if(dimensions() <= 0) {
			std::printf("CScreen::CScreen(): error: invalid dimensions (%d,%d)\n",
				new_width,new_height
			);
			std::exit(-1);
		}

		m_bmp.resize(dimensions());
		clear(fern::CColor(0,0,0));
	}

	auto CScreen::in_range(int x, int y) -> bool {
		if(x < 0 || x >= width()) return false;
		if(y < 0 || y >= height()) return false;
		return true;
	}
	auto CScreen::dot_set(int x, int y, CColor color) -> void {
		if(!in_range(x,y)) {
			std::printf("CScreen::dot_set(): error: invalid coords (%d,%d)\n",
				x,y
			);
			std::exit(-1);
		}
		dot_access(x,y) = color;
	}
	auto CScreen::clear(fern::CColor color) -> void {
		for(int i=0; i<dimensions(); i++) {
			m_bmp[i] = color;
		}
	}
}

