#pragma once
#include <SFML/Graphics.hpp>
#include "../../shared/field_chunks.hpp"
#include "../../shared/field_landmarks.hpp"

// Reuses the supplied Sunny Peak sheet; no world-sized texture is allocated.
class FieldRenderer {
    sf::Texture atlas_;
    sf::Texture stump_;
    mutable field_chunks::Cache cache_{12};
public:
    struct DrawStats { int chunks=0,tiles=0,details=0,landmarks=0; };
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
        sf::Image stump;stump.create(88,47,sf::Color::Transparent);stump.copy(sourceImage,0,0,{362,423,88,47});
        stump.createMaskFromColor(sourceImage.getPixel(340,430));
        if(!stump_.loadFromImage(stump)) return false;
        stump_.setSmooth(false);atlas_.setSmooth(false); cache_.clear(); return true;
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
                if(tile==field_terrain::Tile::Grass && !field_terrain::starter(x,y)) {
                    const sf::Color tones[]={{255,255,255},{247,255,238},{239,250,240},{255,250,226}};
                    for(unsigned i=0;i<4;++i) mesh[mesh.getVertexCount()-1-i].color=tones[field_landmarks::zone(x,y)];
                }
                if(tile==field_terrain::Tile::Bridge && (y%64==31 || y%64==33)) {
                    const float px=(x-left)*50.f,py=(y-top)*50.f+(y%64==31 ? 2.f : 45.f);
                    rail(edges,px,py,50,3); rail(edges,px+3,py-2,5,7);
                }
            }
            if(mesh.getVertexCount()) target.draw(mesh,&atlas_);
            if(edges.getVertexCount()) target.draw(edges);
        }
        // Small authored patches are ground decoration, never collision objects.
        sf::VertexArray flowers(sf::Quads);
        for(int y=top;y<top+16;++y) for(int x=left;x<left+18;++x) {
            const auto detail=field_landmarks::detail(x,y);
            if(!detail) continue;
            quad(flowers,x,y,left,top,detail==1 ? sf::Vector2i{357,8} :
                (detail==2 ? sf::Vector2i{1068,144} : sf::Vector2i{275,60}));++last_.details;
        }
        if(flowers.getVertexCount()) target.draw(flowers,&atlas_);
        for(const auto& landmark:field_landmarks::all) {
            if(!field_landmarks::nearby(landmark,left,top)) continue;
            ++last_.landmarks;
            const float x=(landmark.x-left)*50.f+55,y=(landmark.y-top)*50.f+55;
            sf::RectangleShape post({6,29});post.setPosition(x+22,y+17);post.setFillColor({112,73,38});target.draw(post);
            sf::RectangleShape sign({38,19});sign.setPosition(x+6,y+4);sign.setFillColor({191,140,77});
            sign.setOutlineColor({90,58,32});sign.setOutlineThickness(2);target.draw(sign);
            // One to five pale notches match the map legend without a font dependency.
            for(int n=0;n<landmark.id;++n) {
                sf::RectangleShape notch({3,9});notch.setPosition(x+10+n*6.f,y+9);notch.setFillColor({255,248,228});target.draw(notch);
            }
            // Surround the marker with a different piece of the original ground art.
            if(landmark.id==4) {
                sf::Sprite patch(stump_);patch.setScale(2,2);patch.setPosition(x+48,y+12);target.draw(patch);
            }
        }
    }
};
