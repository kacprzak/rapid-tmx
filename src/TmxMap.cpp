/* -*- c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#include "TmxMap.h"

#include "base64/base64.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

#include <cstring>
#include <iostream>
#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <locale>

// See: https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

static std::vector<unsigned char> decompress(std::string data, int expectedSize = 256);

namespace tmx {

std::vector<Tileset> loadTilesets(rapidxml::xml_node<>* tileset_node);
std::vector<Layer> loadLayers(rapidxml::xml_node<>* layer_node);
std::vector<ObjectGroup> loadObjectGroups(rapidxml::xml_node<>* objectGroup_node);
std::vector<Property> loadProperties(rapidxml::xml_node<>* node);

//------------------------------------------------------------------------------

Map::Map()
    : width(0)
    , height(0)
    , tileWidth(0)
    , tileHeight(0)
{
    //
}

//------------------------------------------------------------------------------

bool Map::loadFromFile(const std::string& filename)
{
    // File has to exist while working with XML
    rapidxml::file<> xmlfile(filename.c_str());

    using namespace rapidxml;

    xml_document<> doc;
    doc.parse<0>(xmlfile.data());

    xml_node<>* map_node = doc.first_node("map");

    try {
        version     = map_node->first_attribute("version")->value();
        orientation = map_node->first_attribute("orientation")->value();
        width       = std::stoi(map_node->first_attribute("width")->value());
        height      = std::stoi(map_node->first_attribute("height")->value());
        tileWidth   = std::stoi(map_node->first_attribute("tilewidth")->value());
        tileHeight  = std::stoi(map_node->first_attribute("tileheight")->value());

        xml_attribute<>* bgColor_attr = map_node->first_attribute("backgroundcolor");
        if (bgColor_attr) backgroundColor = bgColor_attr->value();

        properties = loadProperties(map_node);

        xml_node<>* tileset_node = map_node->first_node("tileset");
        tilesets                 = loadTilesets(tileset_node);

        xml_node<>* layer_node = map_node->first_node("layer");
        layers                 = loadLayers(layer_node);

        xml_node<>* objectGroup_node = map_node->first_node("objectgroup");
        objectGroups                 = loadObjectGroups(objectGroup_node);

    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

std::vector<Tileset> loadTilesets(rapidxml::xml_node<>* tileset_node)
{
    using namespace rapidxml;

    std::vector<Tileset> tilesets;

    while (tileset_node) {
        Tileset tileset;

        tileset.firstGid =
            std::stoul(tileset_node->first_attribute("firstgid")->value());
        tileset.name      = tileset_node->first_attribute("name")->value();
        tileset.tileWidth = std::stoi(tileset_node->first_attribute("tilewidth")->value());
        tileset.tileHeight =
            std::stoi(tileset_node->first_attribute("tileheight")->value());
        tileset.imageSource = tileset_node->first_node("image")->first_attribute("source")->value();

        tileset.spacing               = 0;
        xml_attribute<>* spacing_attr = tileset_node->first_attribute("spacing");
        if (spacing_attr) tileset.spacing = std::stoi(spacing_attr->value());

        tileset.margin               = 0;
        xml_attribute<>* margin_attr = tileset_node->first_attribute("margin");
        if (margin_attr) tileset.margin = std::stoi(margin_attr->value());

        xml_node<>* image_node = tileset_node->first_node("image");
        tileset.imageSource    = image_node->first_attribute("source")->value();
        tileset.imageWidth     = std::stoi(image_node->first_attribute("width")->value());
        tileset.imageHeight    = std::stoi(image_node->first_attribute("height")->value());

        tilesets.push_back(tileset);
        tileset_node = tileset_node->next_sibling("tileset");
    }

    return tilesets;
}

//------------------------------------------------------------------------------

std::vector<Layer> loadLayers(rapidxml::xml_node<>* layer_node)
{
    using namespace rapidxml;

    std::vector<Layer> layers;

    while (layer_node) {
        Layer layer;

        layer.name   = layer_node->first_attribute("name")->value();
        layer.width  = std::stoi(layer_node->first_attribute("width")->value());
        layer.height = std::stoi(layer_node->first_attribute("height")->value());

        layer.visible                 = "1";
        xml_attribute<>* visible_attr = layer_node->first_attribute("visible");
        if (visible_attr) layer.visible = visible_attr->value();

        layer.properties = loadProperties(layer_node);

        // Data loading
        xml_node<>* data_node = layer_node->first_node("data");
        layer.dataEncoding    = data_node->first_attribute("encoding")->value();

        xml_attribute<>* compression_attr = data_node->first_attribute("compression");
        if (compression_attr) layer.compression = compression_attr->value();

        if (layer.dataEncoding == "csv") {

            char* data = data_node->value();

            const char* sep = ", \t\n\r";
            while (*data != '\0') {
                std::size_t pos;
                layer.data.push_back(std::stoul(data, &pos));
                data += pos;
                pos = std::strspn(data, sep);
                data += pos;
            }
        } else if (layer.dataEncoding == "base64") {

            std::string nodevalue = data_node->value();
            trim(nodevalue);

            std::vector<unsigned char> data;
            if (layer.compression.empty()) {
                std::string decoded = base64_decode(nodevalue);
                for (size_t i = 0; i < decoded.size(); ++i) {
                    data.push_back(decoded[i]);
                }
            } else {
                data = decompress(base64_decode(nodevalue));
            }

            for (unsigned i = 0; i < data.size(); i += 4) {
                unsigned global_tile_id =
                    data[i] | data[i + 1] << 8 | data[i + 2] << 16 | data[i + 3] << 24;

                layer.data.push_back(global_tile_id);
            }
        } else {
            std::cerr << "WARNING: Map layer data loading from " << layer.dataEncoding
                      << " is not implemented." << std::endl;
        }

        layers.push_back(layer);
        layer_node = layer_node->next_sibling("layer");
    }

    return layers;
}

//------------------------------------------------------------------------------

std::vector<ObjectGroup> loadObjectGroups(rapidxml::xml_node<>* objectGroup_node)
{
    using namespace rapidxml;

    std::vector<ObjectGroup> objectGroups;

    while (objectGroup_node) {
        ObjectGroup objectGroup;

        objectGroup.name  = objectGroup_node->first_attribute("name")->value();
        objectGroup.width = std::stoi(objectGroup_node->first_attribute("width")->value());
        objectGroup.height =
            std::stoi(objectGroup_node->first_attribute("height")->value());

        objectGroup.properties = loadProperties(objectGroup_node);

        xml_node<>* object_node = objectGroup_node->first_node("object");

        while (object_node) {
            Object object;

            xml_attribute<>* name_attr = object_node->first_attribute("name");
            if (name_attr) object.name = name_attr->value();

            xml_attribute<>* type_attr = object_node->first_attribute("type");
            if (type_attr) object.type = type_attr->value();

            xml_attribute<>* gid_attr = object_node->first_attribute("gid");
            object.gid                = (gid_attr) ? std::stoul(gid_attr->value()) : 0;

            object.x = std::stoi(object_node->first_attribute("x")->value());
            object.y = std::stoi(object_node->first_attribute("y")->value());

            xml_attribute<>* width_attr = object_node->first_attribute("width");
            object.width = (width_attr) ? std::stoul(width_attr->value()) : 0;

            xml_attribute<>* height_attr = object_node->first_attribute("height");
            object.height = (height_attr) ? std::stoul(height_attr->value()) : 0;

            object.visible                = "1";
            xml_attribute<>* visible_attr = object_node->first_attribute("visible");
            if (visible_attr) object.visible = visible_attr->value();

            object.properties = loadProperties(object_node);

            xml_node<>* shape_node;
            bool readPoints = false;
            if ((shape_node = object_node->first_node("ellipse"))) {
                object.shape = "ellipse";
            } else if ((shape_node = object_node->first_node("polygon"))) {
                object.shape = "polygon";
                readPoints   = true;
            } else if ((shape_node = object_node->first_node("polyline"))) {
                object.shape = "polyline";
                readPoints   = true;
            }

            if (readPoints) {
                char* points = shape_node->first_attribute("points")->value();
                const char* sep = ", ";

                while(*points != '\0') {
                    std::size_t pos;

                    int x = std::stoi(points, &pos);
                    points += pos;
                    pos = std::strspn(points, sep);
                    points += pos;

                    int y = std::stoi(points, &pos);
                    points += pos;
                    pos = std::strspn(points, sep);
                    points += pos;

                    object.points.push_back(std::pair<int, int>(x, y));
                }
            }

            objectGroup.objects.push_back(object);
            object_node = object_node->next_sibling("object");
        }

        objectGroups.push_back(objectGroup);
        objectGroup_node = objectGroup_node->next_sibling("objectgroup");
    }

    return objectGroups;
}

//------------------------------------------------------------------------------

std::vector<Property> loadProperties(rapidxml::xml_node<>* node)
{
    using namespace rapidxml;

    std::vector<Property> properties;

    xml_node<>* properties_node = node->first_node("properties");
    if (properties_node) {
        xml_node<>* property_node = properties_node->first_node("property");
        while (property_node) {
            Property prop;
            prop.name  = property_node->first_attribute("name")->value();
            prop.value = property_node->first_attribute("value")->value();

            properties.push_back(prop);

            property_node = property_node->next_sibling("property");
        }
    }

    return properties;
}

//------------------------------------------------------------------------------

const Tileset* Map::tilesetForTile(unsigned gid) const
{
    for (int i = tilesets.size() - 1; i >= 0; --i) {
        const tmx::Tileset& tileset = tilesets[i];

        if (tileset.firstGid <= gid) {
            return &tileset;
        }
    }

    // Should never happen
    std::cerr << "ERROR: Unable to find tileset for tile #" << gid << "\n";
    return nullptr;
}

} // namespace tmx

//==============================================================================

static void logZlibError(int error)
{
    switch (error) {
    case Z_MEM_ERROR: std::cerr << "Out of memory while (de)compressing data!\n"; break;
    case Z_VERSION_ERROR: std::cerr << "Incompatible zlib version!\n"; break;
    case Z_NEED_DICT:
    case Z_DATA_ERROR: std::cerr << "Incorrect zlib compressed data!\n"; break;
    default: std::cerr << "Unknown error while (de)compressing data!\n";
    }
}

//------------------------------------------------------------------------------

// gzip or zlib
static std::vector<unsigned char> decompress(std::string data, int expectedSize)
{
    std::vector<Bytef> out(expectedSize);
    const Bytef* in = reinterpret_cast<const Bytef*>(data.c_str());

    z_stream strm;

    strm.zalloc    = Z_NULL;
    strm.zfree     = Z_NULL;
    strm.opaque    = Z_NULL;
    strm.next_in   = (Bytef*)in;
    strm.avail_in  = data.length();
    strm.next_out  = (Bytef*)out.data();
    strm.avail_out = out.size();

    int ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK) {
        logZlibError(ret);
        return std::vector<unsigned char>();
    }

    do {
        ret = inflate(&strm, Z_SYNC_FLUSH);

        switch (ret) {
        case Z_NEED_DICT:
            /* FALLTHRU */
        case Z_STREAM_ERROR:
            ret = Z_DATA_ERROR;
            /* FALLTHRU */
        case Z_DATA_ERROR:
            /* FALLTHRU */
        case Z_MEM_ERROR:
            inflateEnd(&strm);
            logZlibError(ret);
            return std::vector<unsigned char>();
        }

        if (ret != Z_STREAM_END) {
            int oldSize = out.size();
            out.resize(out.size() * 2);

            strm.next_out  = (Bytef*)(out.data() + oldSize);
            strm.avail_out = oldSize;
        }
    } while (ret != Z_STREAM_END);

    if (strm.avail_in != 0) {
        logZlibError(Z_DATA_ERROR);
        return std::vector<unsigned char>();
    }

    const int outLength = out.size() - strm.avail_out;
    inflateEnd(&strm);

    out.resize(outLength);
    return out;
}
