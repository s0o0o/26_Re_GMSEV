#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/field_chunks.hpp"

// Reuses the supplied Sunny Peak sheet; no world-sized texture is allocated.
class FieldRenderer {
    sf::Texture atlas_;
    mutable field_chunks::Cache cache_{12};
public:
    struct DrawStats { int chunks=0,tiles=0; };
private:
    mutable DrawStats last_;
    static sf::Vector2i source(field_terrain::Tile tile,int x,int y) {
        using field_terrain::Tile;
        switch(tile) {
        case Tile::Trail: return {1680,185};
        case Tile::Water: return {280,150};
        case Tile::Cliff:
            return {950+(x%4)*16,y>=128 && y<=130 ? 220+(y-128)*16 : 236};
        case Tile::Bridge: return {1040,708};
        default: return {1050,145};
        }
    }
    static void quad(sf::VertexArray& mesh,int x,int y,int left,int top,sf::Vector2i uv) {
        const float px=(x-left)*50.f,py=(y-top)*50.f;
        const float u=static_cast<float>(uv.x),v=static_cast<float>(uv.y);
        mesh.append({{px,py},{u,v}}); mesh.append({{px+50,py},{u+16,v}});
        mesh.append({{px+50,py+50},{u+16,v+16}}); mesh.append({{px,py+50},{u,v+16}});
    }
    static void rail(sf::VertexArray& mesh,float x,float y,float w,float h) {
        const sf::Color wood(112,73,38);
        mesh.append({{x,y},wood}); mesh.append({{x+w,y},wood});
        mesh.append({{x+w,y+h},wood}); mesh.append({{x,y+h},wood});
    }
public:
    bool load(const sf::Image& sourceImage) {
        if(sourceImage.getSize().x<1696 || sourceImage.getSize().y<724 || !atlas_.loadFromImage(sourceImage)) return false;
        atlas_.setSmooth(false); cache_.clear(); return true;
    }
    DrawStats lastDraw() const { return last_; }
    field_chunks::Stats cacheStats() const { return cache_.stats(); }
    void draw(sf::RenderTarget& target,int left,int top) const {
        last_={};
        const auto bounds=field_chunks::visible(left,top);
        for(int cy=bounds.top;cy<=bounds.bottom;++cy) for(int cx=bounds.left;cx<=bounds.right;++cx) {
            ++last_.chunks; sf::VertexArray mesh(sf::Quads),edges(sf::Quads);
            const int beginX=(std::max)(left,cx*16),beginY=(std::max)(top,cy*16);
            const int endX=(std::min)({left+18,(cx+1)*16,world_regions::fieldWidth});
            const int endY=(std::min)({top+16,(cy+1)*16,world_regions::fieldHeight});
            for(int y=beginY;y<endY;++y) for(int x=beginX;x<endX;++x) {
                // Preserve the complete authored welcome screen, except its east exit.
                if(field_terrain::starter(x,y) && !(x>=17 && y>=8 && y<=10)) continue;
                const auto tile=cache_.at({world_regions::Id::SunnyPeak,x,y});
                quad(mesh,x,y,left,top,source(tile,x,y)); ++last_.tiles;
                if(tile==field_terrain::Tile::Bridge && (y%64==31 || y%64==33)) {
                    const float px=(x-left)*50.f,py=(y-top)*50.f+(y%64==31 ? 2.f : 45.f);
                    rail(edges,px,py,50,3); rail(edges,px+3,py-2,5,7);
                }
            }
            if(mesh.getVertexCount()) target.draw(mesh,&atlas_);
            if(edges.getVertexCount()) target.draw(edges);
        }
    }
};
