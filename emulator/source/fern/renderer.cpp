#include <fern.h>
#include <SDL2/SDL.h>

namespace fern {
	CRenderer::CRenderer() 
	: m_screen(160,144) {
		m_window = nullptr;
	}
	CRenderer::~CRenderer() {
		
	}

	auto CRenderer::window_create() -> void {
		SDL_CreateWindowAndRenderer(
			160,144,0,
			&m_window,
			&m_renderer
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
		const fern::CColor palet_gray[4] = {
			fern::CColor(255,255,255),
			fern::CColor(192,192,192),
			fern::CColor(112,112,112),
			fern::CColor(12,12,12)
		};
		int bgp_table[4] = {};
		auto& mem = emu()->mem;
		const int bgp = mem.m_io.m_BGP;

		// create bgp lookup table
		auto bgp_shifter = bgp;
		for(int i=0; i<4; i++) {
			bgp_table[i] = bgp_shifter & 3;
			bgp_shifter >>= 2;
		}

		// render to screen
		const size_t addr_chrbase = 0x1800;
		const size_t addr_mapbase = 0x1800;
		const size_t addr_mapline = addr_mapbase + ((draw_y/8) * 0x20);

		for(int draw_x=0; draw_x<160; draw_x++) {
			int fetch_x = draw_x;
			// fetch tile
			int tile = static_cast<int8_t>(mem.m_vram.at(addr_mapline + (fetch_x/8)));
			tile -= 0x80;
			int tileaddr = addr_chrbase + tile * 0x10;
			// get pixel
			tileaddr += (draw_y&7)*2;
			int lineA = mem.m_vram.at(tileaddr);
			int lineB = mem.m_vram.at(tileaddr+1);
			int dotA = (lineA >> (7-(fetch_x&7))) & 1;
			int dotB = (lineB >> (7-(fetch_x&7))) & 1;
			int dot = dotA | (dotB<<1);
			m_screen.dot_set(draw_x,draw_y,palet_gray[bgp_table[dot]]);
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

