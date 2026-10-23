Warning: truncated output (original token count: 51401)
Total output lines: 4181

// The renderer's implementation: resources, materials, scene, meshes, animation, particles, camera.
// Split from renderer.cpp; the shared
// declarations are in renderer_impl.h.
#include "formats/dds.h"
#include "core/touch_pad.h"
#include "render/menu_chrome.h"
#include "render/renderer_impl.h"

namespace tl::render {

namespace renderer_detail {
/// Parse the script that declares `material` wherever it is in the archive
/// (renderer_archive.cpp).
void parse_material_declaring(const std::string& material);
/// `CMeshListener`, installed on the mesh manager once the archives are mounted.
Ogre::MeshSerializerListener& game_mesh_listener();
}  // namespace renderer_detail
Renderer::Renderer() : impl_(std::make_unique<Impl>()) { impl_->wire_bag_item_events(); }

Renderer::~Renderer() { shutdown(); }

namespace renderer_detail {
namespace {
/// `CMeshListener` (Runic's `Ogre::MeshSerializerListener`), its skeleton half.
struct GameMeshListener final : Ogre::MeshSerializerListener {
    // `processMaterialName` is the material scripts' naming,
    // which the port does where it parses them (`parse_material_script`).
    // PORT: a material the mesh's own directory does not declare is looked for
    // in every script before OGRE gives up on it (parse_material_declaring).
    void processMaterialName(Ogre::Mesh*, Ogre::String* name) override {
        if (name == nullptr || name->empty()) return;
        if (Ogre::MaterialManager::getSingleton().resourceExists(*name, kGameGroup)) return;
        parse_material_declaring(*name);
    }

    // `CMeshListener::processSkeletonName`: the mesh's directory
    // (`FILESYSTEM::RemoveFileName` of its name, cut to start at `MEDIA`) in
    // front of the name the mesh records. So `media/models/NPCs/Brink/
    // Merc.MESH` asks for its own `Merc.SKELETON` and never the one of the same
    // name in `media/models/Mercenary/` - which the archive index, keyed by
    // basename, cannot tell apart. The port's mesh names start at `media/`
    // already; the archive's own spelling of the joined path is used, as
    // `archive_path_in_dir` gives it, so the lookup does not hang on case.
    void processSkeletonName(Ogre::Mesh* mesh, Ogre::String* name) override {
        if (mesh == nullptr || name == nullptr || name->empty()) return;
        const std::string& path = mesh->getName();
        const size_t slash = path.find_last_of("/\\");
        if (slash == std::string::npos) return;
        const std::string dir = path.substr(0, slash + 1);
        const std::string spelled = archive_path_in_dir(*name, dir);
        *name = spelled.empty() ? dir + *name : spelled;
    }

    void processMeshCompleted(Ogre::Mesh*) override {}
};
}  // namespace

Ogre::MeshSerializerListener& game_mesh_listener() {
    static GameMeshListener listener;
    return listener;
}

/// Parse one material script, at most once per run.
///
/// Re-parsing is not merely wasteful: OGRE throws ItemIdentityException on a
/// duplicate material name, so a script parsed twice would raise an error that
/// looks like corrupt data.
/// The names a material script declares, in order.
///
/// Only top-level `material` blocks count, so brace depth has to be tracked - a
/// script's passes and texture units are full of words that would otherwise
/// look like declarations.
std::vector<std::string> declared_materials(const std::string& text) {
    std::vector<std::string> names;
    int depth = 0;
    size_t pos = 0;

    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(pos, end - pos);
        pos = end + 1;

        const size_t comment = line.find("//");
        if (comment != std::string::npos) line.resize(comment);

        size_t first = line.find_first_not_of(" \t\r");
        if (depth == 0 && first != std::string::npos && line.compare(first, 9, "material ") == 0) {
            size_t name_start = line.find_first_not_of(" \t", first + 9);
            if (name_start != std::string::npos) {
                size_t name_end = line.find_first_of(" \t:{\r", name_start);
                if (name_end == std::string::npos) name_end = line.size();
                names.push_back(line.substr(name_start, name_end - name_start));
            }
        }

        for (char c : line) {
            if (c == '{') ++depth;
            else if (c == '}' && depth > 0) --depth;
        }
    }
    return names;
}

/// Rewrite this script's own `material <Name>` declarations to
/// `material <Name>@<dir>`, for exactly the names in `renames`.
///
/// Same line-and-brace walk as declared_materials(), because that is the
/// grammar this has to agree with: a name that function would not have
/// counted as a top-level declaration must not be renamed here either. Only
/// the declaration line changes - Torchlight's own material scripts never
/// reference one material from within another, so nothing else in the file
/// names it.
std::string qualify_material_names(
    const std::string& text, const std::vector<std::pair<std::string, std::string>>& renames) {
    if (renames.empty()) return text;
    std::string out;
    out.reserve(text.size());
    int depth = 0;
    size_t pos = 0;

    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        const bool has_nl = end != std::string::npos;
        if (!has_nl) end = text.size();
        std::string line = text.substr(pos, end - pos);
        pos = has_nl ? end + 1 : end;

        size_t first = line.find_first_not_of(" \t\r");
        if (depth == 0 && first != std::string::npos && line.compare(first, 9, "material ") == 0) {
            size_t name_start = line.find_first_not_of(" \t", first + 9);
            if (name_start != std::string::npos) {
                size_t name_end = line.find_first_of(" \t:{\r", name_start);
                if (name_end == std::string::npos) name_end = line.size();
                const std::string name = line.substr(name_start, name_end - name_start);
                for (const auto& rename : renames) {
                    if (rename.first == name) {
                        line.replace(name_start, name_end - name_start, rename.second);
                        break;
                    }
                }
            }
        }

        for (char c : line) {
            if (c == '{') ++depth;
            else if (c == '}' && depth > 0) --depth;
        }
        out += line;
        if (has_nl) out += '\n';
    }
    return out;
}

/// Remove this script's own `material <Name> { ... }` blocks entirely, for
/// exactly the names in `drop`.
///
/// For a name that has already been aliased once on this same directory's
/// behalf - a second `collision_*.material` beside the first, both naming
/// the generic `colliders/nocollide` - qualifying it again would declare
/// `colliders/nocollide@<dir>` a second time, which is the identical
/// "material with this name already exists" failure one level down. The
/// first file's alias already speaks for the directory, so a repeat is
/// dropped rather than requalified.
///
/// Same line-and-brace walk as qualify_material_names(), extended to track
/// the whole span of a matching block rather than only its first line.
std::string strip_material_blocks(const std::string& text, const std::vector<std::string>& drop) {
    if (drop.empty()) return text;
    std::string out;
    out.reserve(text.size());
    int depth = 0;
    bool dropping = false;
    size_t pos = 0;

    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        const bool has_nl = end != std::string::npos;
        if (!has_nl) end = text.size();
        const std::string line = text.substr(pos, end - pos);
        pos = has_nl ? end + 1 : end;

        if (!dropping && depth == 0) {
            size_t first = line.find_first_not_of(" \t\r");
            if (first != std::string::npos && line.compare(first, 9, "material ") == 0) {
                size_t name_start = line.find_first_not_of(" \t", first + 9);
                if (name_start != std::string::npos) {
                    size_t name_end = line.find_first_of(" \t:{\r", name_start);
                    if (name_end == std::string::npos) name_end = line.size();
                    const std::string name = line.substr(name_start, name_end - name_start);
                    if (std::find(drop.begin(), drop.end(), name) != drop.end()) dropping = true;
                }
            }
        }

        const bool was_dropping = dropping;
        for (char c : line) {
            if (c == '{') ++depth;
            else if (c == '}' && depth > 0) {
                --depth;
                if (dropping && depth == 0) dropping = false;
            }
        }
        if (was_dropping) continue;  // every line of the dropped block, closer included

        out += line;
        if (has_nl) out += '\n';
    }
    return out;
}

/// Rewrite a material script so no material is declared twice.
///
/// Two different collisions happen, and only one of them is between files:
///
///  * The same name appears in two scripts - `town_enchanter02_shadow` is in
///    several - so a script re-declares something already loaded.
///  * **The same name appears twice in one script.**
///    `menu_billboard_backdrop.material` declares
///    `Material_#238684871/town_enchanter02_shadow` at blocks 2 and 5 of 7.
///
/// Either way OGRE throws ItemIdentityException and abandons the *entire*
/// script, so everything after the collision is lost - which is how the menu's
/// backdrop ended up a grey slab: the collision was on an unrelated material
/// three blocks above `billboard_town_mat`, the one the mesh actually needs.
///
/// Keeping the first declaration and dropping later ones matches what the
/// original engine must do, since these files ship as they are and the game
/// renders them.
// PORT: material scripts, a later declaration of a name dropped (OGRE loading).
std::string deduplicate_material_script(const std::string& text, size_t& dropped) {
    std::string out;
    out.reserve(text.size());

    std::set<std::string> seen;
    int depth = 0;
    bool skipping = false;
    bool entered_block = false;
    size_t pos = 0;
    dropped = 0;

    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        const std::string line = text.substr(pos, end - pos);
        pos = end + 1;

        std::string code = line;
        const size_t comment = code.find("//");
        if (comment != std::string::npos) code.resize(comment);

        if (depth == 0 && !skipping) {
            const size_t first = code.find_first_not_of(" \t\r");
            if (first != std::string::npos && code.compare(first, 9, "material ") == 0) {
                const size_t name_start = code.find_first_not_of(" \t", first + 9);
                if (name_start != std::string::npos) {
                    size_t name_end = code.find_first_of(" \t:{\r", name_start);
                    if (name_end == std::string::npos) name_end = code.size();
                    const std::string name = code.substr(name_start, name_end - name_start);
                    if (!seen.insert(name).second) {
                        skipping = true;
                        entered_block = false;
                        ++dropped;
                    }
                }
            }
        }

        if (!skipping) out += line + "\n";

        for (char c : code) {
            if (c == '{') ++depth;
            else if (c == '}' && depth > 0) --depth;
        }

        if (skipping) {
            if (depth > 0) entered_block = true;
            else if (entered_block) skipping = false;
        }
    }
    return out;
}

/// Supply the sources OGRE 1.6 defaulted for a bare `colour_op_ex`.
///
/// Two shipped scripts - `media/levelSets/crypt/crypt_water_01.material` and
/// `media/levelSets/palace/palace_water.material` - write
///
///     colour_op_ex modulate_x4
///
/// with no source arguments. Torchlight's OGRE accepted it; OGRE 14's script
/// compiler does not, and rejects the line with "colour_op_ex must have at
/// least 3 arguments", leaving that texture layer blended as though the
/// directive were absent.
///
/// The values to fill in are not a guess: `TextureUnitState::setColourOperationEx`
/// declares `source1 = LBS_TEXTURE, source2 = LBS_CURRENT` as its defaults, and
/// the script form is a thin wrapper over that call - so a script omitting them
/// meant exactly `src_texture src_current`.
///
/// Done here rather than as a patch to `external/ogre` deliberately: this is not
/// a Switch workaround, it is our reader being compatible with the data, and the
/// project's rule is that OGRE is never edited in place.
std::string complete_colour_op_ex(const std::string& text, size_t& fixed) {
    fixed = 0;
    std::string out;
    out.reserve(text.size());

    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find('\n', start);
        const bool last = (end == std::string::npos);
        if (last) end = text.size();
        std::string line = text.substr(start, end - start);

        // Tokenise, ignoring a trailing carriage return.
        std::string body = line;
        while (!body.empty() && (body.back() == '\r')) body.pop_back();

        size_t first = body.find_first_not_of(" \t");
        if (first != std::string::npos) {
            const std::string trimmed = body.substr(first);
            if (trimmed.compare(0, 13, "colour_op_ex ") == 0 ||
                trimmed.compare(0, 13, "color_op_ex  ") == 0) {
                size_t tokens = 0;
                size_t at = 0;
                while (at < trimmed.size()) {
                    while (at < trimmed.size() && (trimmed[at] == ' ' || trimmed[at] == '\t')) ++at;
                    if (at >= trimmed.size()) break;
                    ++tokens;
                    while (at < trimmed.size() && trimmed[at] != ' ' && trimmed[at] != '\t') ++at;
                }
                if (tokens == 2) {  // the directive plus its operation, and nothing else
                    line = body + " src_texture src_current";
                    ++fixed;
                }
            }
        }

        out += line;
        if (!last) out += '\n';
        if (last) break;
        start = end + 1;
    }
    return out;
}

/// Material scripts already parsed into the current OGRE session.
///
/// File scope rather than a function-local static so that it can be forgotten
/// from `mount_resources()`; see the note there.
std::set<std::string> g_parsed_materials;

/// How many times each script has been parsed, so a storm is visible.
///
/// **Material names collide between scripts, and re-parsing on that basis does
/// not work.** The obvious repair for a name that has changed hands is to
/// re-parse whichever script wants it back - but scripts in one directory
/// declare overlapping names, `parse_sibling_material()` parses a whole
/// directory when a mesh has no script of its own, and each parse invalidates
/// the others. Making the cache a freshness check turned the town's 349 parses
/// into **10,637**, every one of them redefining, and the console never
/// finished loading the level. The desktop did, which is exactly why the log
/// grew from 600 lines to 13,000 and the run still exited zero.
///
/// So the cache stays a gate, and a caller that genuinely needs a script's own
/// definitions back asks for them - see the `force` argument, which only the
/// character-creation swap uses.
std::map<std::string, size_t> g_parse_counts;

/// Which directory currently owns a plain (unqualified) material name.
///
/// Most redefinitions are the same asset coming back - a level transition
/// reusing the menu's own names, or the character-creation swap asking for
/// its own wardrobe piece back - and those are meant to clobber: the newer
/// script IS the current truth for that name. But two *unrelated* meshes can
/// land on the same generic exporter name by accident - `media/models/NPCs/
/// Mage/Mage.material` and `.../Mage2/Mage.material` both declare
/// `Standard_20`, one green and one red, because that is what the export tool
/// calls an untitled material. Clobbering there does not update stale state,
/// it destroys information: whichever NPC is placed - or whose script parses
/// - second silently repaints the first. See qualify_cross_directory_names().
std::map<std::string, std::string> g_material_home;

/// A material name a cross-directory collision could not be allowed to
/// clobber, so it was parsed under a qualified alias instead. Keyed by
/// `<placing directory>\x1f<name as the .MESH file embeds it>`, valued by the
/// alias actually registered with OGRE's MaterialManager. Consulted once per
/// placed entity, in place_mesh().
std::map<std::string, std::string> g_material_alias;

/// Which script currently owns a plain material name, so that two scripts of
/// one directory can be put in the original's order. See the note on
/// `material_script_rank()`.
std::map<std::string, std::string> g_material_script;

void forget_parsed_materials() {
    g_parsed_materials.clear();
    g_parse_counts.clear();
    g_material_home.clear();
    g_material_alias.clear();
    g_material_script.clear();
}

/// The scene a material fix has to reach.
///
/// File scope for the same reason `g_parsed_materials` is: parsing is done by
/// free functions on purpose, and a redefinition there invalidates entities
/// anywhere in the scene, not only the ones the caller happens to hold.
Ogre::SceneManager* g_material_scene = nullptr;

/// `force` re-parses a script that is already in the cache, so that its own
/// definitions take back the names it declares.
///
/// Only the character-creation swap passes it. It is not a general repair: see
/// the note on g_parse_counts for what happens when re-parsing is driven by
/// name ownership instead of by an explicit caller.
void parse_material_script(const std::string& script, std::vector<std::string>* redefined,
                           bool force) {
    if (!g_parsed_materials.insert(script).second && !force) return;
    const size_t parses = ++g_parse_counts[script];
    if (parses == kNoisyParseCount) {
        TL_LOG_WARN(
            "material script '%s' has been parsed %zu times; something is thrashing the "
            "cache - see g_parse_counts",
            script.c_str(), parses);
    }

    auto& groups = Ogre::ResourceGroupManager::getSingleton();
    auto& materials = Ogre::MaterialManager::getSingleton();
    try {
        Ogre::DataStreamPtr stream = groups.openResource(script, kGameGroup);
        const std::string raw = stream->getAsString();

        size_t dropped = 0;
        std::string text = deduplicate_material_script(raw, dropped);

        size_t completed = 0;
        text = complete_colour_op_ex(text, completed);
        if (completed > 0) {
            TL_LOG_INFO("  %zu bare colour_op_ex completed with OGRE's own defaults in %s",
                        completed, script.c_str());
        }

        size_t filled = 0;
        text = fill_empty_texture_units(text, script, filled);
        if (filled > 0) {
            TL_LOG_INFO("  %zu empty texture_unit(s) in %s filled from the set's own artwork",
                        filled, script.c_str());
        }

        // Now the between-file collisions. Removing is safe: an entity already
        // built holds a shared_ptr to the material it was created with, and the
        // new definition serves everything created afterwards.
        //
        // **The shader generator has to be told, and this is the only place
        // that knows.** The RTShaderSystem keeps a table of the shader
        // techniques it has built, keyed by material name and group, and
        // `createShaderBasedTechnique()` refuses to build a second one for a
        // name it already has. Removing the material here and re-parsing makes
        // a *new* `Material` with no shader technique on it, while that table
        // still holds an entry - pointing at a technique that no longer exists.
        // The next time anything draws with it,
        // `ShaderResolver::handleSchemeNotFound` cannot make a technique, the
        // fixed-function one is used instead, and on a render system that has
        // no fixed-function pipeline - both of this port's - that is fatal:
        //
        //     RenderSystem does not support FixedFunction, but technique of
        //     'mine_lantern01' has no Vertex Shader. Use the RTSS
        //
        // The town's material scripts redefine 23 names the main menu
        // backdrop's already declared, so it is the level transition that
        // exposes this; but the same removal happens *within* a single level
        // load - the town redefines 20 of its own - so this was latent all
        // along and only survived because nothing had yet drawn with the
        // stale entry.
        //
        // Through the singleton rather than a member: this is a free function
        // deliberately, and the generator is a process-wide singleton anyway.
        size_t replaced = 0;
        auto* shaders = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        const std::string script_dir = dir_of_asset(script);
        std::vector<std::pair<std::string, std::string>> aliased;
        std::vector<std::string> dropped_duplicate_alias;
        std::vector<std::string> kept_by_earlier;
        for (const std::string& name : declared_materials(text)) {
            if (materials.resourceExists(name, kGameGroup)) {
                const auto home = g_material_home.find(name);
                if (home != g_material_home.end() && home->second != script_dir) {
                    // A different, unrelated directory already owns this
                    // name - two NPC skins sharing an exporter's default
                    // "Standard_20", not the same asset coming back. Removing
                    // the existing one would make it as dead as the RTSS
                    // problem this whole function exists to avoid, just for
                    // colour instead of a crash: the earlier NPC would follow
                    // whichever redefines the name last. Parsed under an
                    // alias instead, and place_mesh() re-points only the
                    // entities actually built from this directory at it.
                    //
                    // **A second file in this same directory can hit the very
                    // same collision.** `media/levelSets/props/lava_props/`
                    // alone has a dozen `collision_*.material` scripts that
                    // all declare the generic `colliders/nocollide`, and every
                    // one of them collides with whatever directory got that
                    // name first. The first file already aliased it on this
                    // directory's behalf; declaring the alias a second time is
                    // the identical "already exists" failure one name over, so
                    // a repeat is dropped instead of requalified.
                    const std::string alias_key = script_dir + "\x1f" + name;
                    if (g_material_alias.count(alias_key) != 0) {
                        dropped_duplicate_alias.push_back(name);
                    } else {
                        const std::string qualified = name + "@" + script_dir;
                        aliased.emplace_back(name, qualified);
                        g_material_alias[alias_key] = qualified;
                    }
                    continue;
                }
                // **Two scripts of one directory: the one earlier in the
                // archive keeps the name.** Runic's OGRE parses a directory's
                // scripts together, in archive order, the first time anything
                // in it is looked up (`ResourceGroupManager::
                // addScriptForParsingLater` 0x25e5a0 keys them by directory,
                // `parseCachedScripts` 0x25db50 runs them from
                // `ResourceManager::getByName` 0x267470, libOgreMain-1.6.5),
                // and names them `<DIRECTORY>/<name>`
                // (`CScriptListener::createObject`, `CMeshListener::processMaterialName`).
                // A later script's `create` of a taken name throws,
                // and `MaterialTranslator::translate` catches it,
                // logs "failed to find or create material" and skips just that
                // material. So `menu_billboard_sky.material`'s scrolling
                // `billboard_wind*_mat` survive `menu_billboard_terrain2
                // .material`'s still copies. The port parses per mesh, in
                // placement order, so the order is restored here by rank.
                // `force` (the character-creation swap) still takes names back.
                const auto owner = g_material_script.find(name);
                if (!force && owner != g_material_script.end() && owner->second != script &&
                    material_script_rank(owner->second) < material_script_rank(script)) {
                    kept_by_earlier.push_back(name);
                    continue;
                }
                if (redefined != nullptr) redefined->push_back(name);
                if (shaders != nullptr) {
                    shaders->removeAllShaderBasedTechniques(name, kGameGroup);
                }
                materials.remove(name, kGameGroup);
                ++replaced;
            }
            g_material_home[name] = script_dir;
            g_material_script[name] = script;
        }
        if (!aliased.empty()) {
            text = qualify_material_names(text, aliased);
            TL_LOG_INFO(
                "  %zu material name(s) in %s collide with another directory's; "
                "parsed under an alias instead of replacing them",
                aliased.size(), script.c_str());
        }
        if (!dropped_duplicate_alias.empty()) {
            text = strip_material_blocks(text, dropped_duplicate_alias);
        }
        if (!kept_by_earlier.empty()) {
            text = strip_material_blocks(text, kept_by_earlier);
            TL_LOG_INFO(
                "  %zu material name(s) in %s are an earlier script's in the archive; "
                "kept theirs",
                kept_by_earlier.size(), script.c_str());
        }

        // This script now owns every name it declares, and whoever owned them
        // before does not - which is what makes them re-parse if they are asked
        // for again.
        // **Everything already built with one of those names is now holding a
        // dead object**, and the RTShaderSystem will not serve it - so this has
        // to reach the whole scene, not just whatever the caller is assembling.
        // The town's own scripts redefine 23 names the menu backdrop already
        // declared and 20 of each other's, so this is the ordinary case rather
        // than the exceptional one.
        //
        // **Must run after parseScript(), not before.** `materials.remove()`
        // above only vacates the name; rebind_scene_materials() re-resolves
        // every stale submesh by looking the name up again with
        // `getByName()`, and between the removal and the parse below that
        // lookup finds nothing to point at - `rebind_materials()` treats a
        // null `current` as "nothing to do" and silently leaves the entity on
        // its now-orphaned `Material`. A name a floor's props redefine only
        // once survives on luck: some *other*, later redefinition's own sweep
        // eventually catches the strays as a side effect. A name that is
        // never redefined again after its own last rewrite - `mine_supports`,
        // rewritten six times running by six different mine-prop scripts on
        // a three-chunk mines floor, with nothing after it to trigger one
        // more sweep - never gets rescued: the stray entities keep pointing
        // at a `Material` object the manager no longer owns, RTShaderSystem
        // still has recorded as *someone else's* technique, and the first
        // frame that actually draws one throws
        // `RenderSystem does not support FixedFunction, but technique of
        // 'mine_supports' has no Vertex Shader` - uncaught, since the live
        // game loop draws every frame outside any try/catch.
        Ogre::DataStreamPtr copy(
            new Ogre::MemoryDataStream(const_cast<char*>(text.data()), text.size(), false, true));
        materials.parseScript(copy, kGameGroup);

        // Every material this parse just declared gets its textures qualified
        // right here, not only the one name whichever caller happens to ask
        // about afterwards by its own mesh's submaterial name.
        //
        // **Without this, a material reached only as a side effect of another
        // mesh's directory-wide scan sits with its bare, unqualified
        // `texture_unit` names indefinitely** - parse_sibling_material()'s
        // fallback parses every script in a mesh's directory looking for the
        // *one* material that mesh's own submesh actually names, and nothing
        // has ever qualified the rest. Usually harmless, because most of
        // those other materials belong to meshes that get placed later and
        // are qualified then, under their own name. Not harmless when a
        // mesh's own submaterial name does not literally match its script's
        // filename - `Mine_railing_large.material` can declare a material
        // that some *other* prop's submesh uses by name, while
        // `mine_railing01.material`'s own material is never asked for by
        // that exact name at all - which is why this looked seed-dependent:
        // whether the file holding a given texture reference got a qualifying
        // caller at all depended on which room pieces the generator happened
        // to pick, not on anything about the reference itself.
        for (const std::string& name : declared_materials(text)) {
            qualify_texture_paths(materials.getByName(name, kGameGroup), script_dir, script_dir);
        }

        // The re-pointing, after the names above: a material handed to a
        // submesh is loaded there and then, and one still carrying its bare
        // `texture miscellaneous.png` logged "Texture layer will be blank" for
        // every unit before the loop above put the full path in.
        if (replaced > 0 && g_material_scene != nullptr) {
            const size_t rebound = rebind_scene_materials(g_material_scene);
            if (rebound > 0) {
                TL_LOG_INFO("  %zu submesh(es) re-pointed after %s redefined %zu name(s)", rebound,
                            script.c_str(), replaced);
            }
        }

        if (dropped > 0 || replaced > 0) {
            TL_LOG_INFO("parsed material script %s (%zu duplicate(s) dropped, %zu redefined)",
                        script.c_str(), dropped, replaced);
        } else {
            TL_LOG_INFO("parsed material script %s", script.c_str());
        }
    } catch (const Ogre::Exception& e) {
        TL_LOG_WARN("could not parse '%s': %s", script.c_str(), e.getDescription().c_str());
    }
}

/// The one texture in `dir` that belongs to this set and this slot, or empty.
/// See fill_empty_texture_units() for where the rule comes from.
std::vector<std::string> underscore_words(const std::string& name) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : name) {
        if (c == '_') {
            words.push_back(word);
            word.clear();
        } else {
            word.push_back(c);
        }
    }
    words.push_back(word);
    return words;
}

std::string wardrobe_texture_for(const std::string& mesh_name, const std::string& dir,
                                 const std::string& slot, const std::string& set_word) {
    (void)mesh_name;
    if (set_word.empty() || dir.empty() || slot.empty()) return {};

    // The slot as a whole trailing word, with the other number allowed:
    // `shoulders` is stored as `..._Shoulder`.
    std::vector<std::string> endings{slot};
    if (!slot.empty() && slot.back() == 's') {
        endings.push_back(slot.substr(0, slot.size() - 1));
    } else {
        endings.push_back(slot + "s");
    }

    // **`Body` is the chest slot's other authored name, in exactly one set.**
    //
    // This is a reading of the item data, which is where the original gets a
    // worn set's texture from in the first place - see the note above
    // fill_empty_texture_units(). Every armour item carries one `[WARDROBE]`
    // group per class naming a `MESH` and a `TEXTURE`, and across all 3,081
    // item files those name 34 wardrobe meshes and 102 textures: three per set,
    // one each for boots, chest and gloves, spelled with the slot's own word.
    // The single exception is `vanquisher_assassin_set`, whose chest items say
    //
    //     [WARDROBE]
    //       CLASS:VANQUISHER
    //       MESH:media/wardrobe/vanquisher/vanquisher_assassin_set.mesh
    //       TEXTURE:media/wardrobe/vanquisher/vanq_assassin_body.png
    //
    // and there is no `Vanq_Assassin_Chest.png` in the archive at all. Without
    // this the assassin set drew a flat yellow torso.
    //
    // Only for `chest`, and only as a *fallback* after the slot's own word
    // finds nothing: `body.png` on its own is the character's bare skin, and
    // letting boots or gloves fall through to it would dress them in it.
    std::vector<std::vector<std::string>> rounds{endings};
    if (slot == "chest") rounds.push_back({"body"});

    // Whole words, not substrings. `plate` is a substring of `heavyplate`, and
    // matching loosely made the Destroyer's plate boots ambiguous against his
    // heavy-plate boots - two candidates, so the scan gave up and the boots
    // stayed flat colour.
    for (const std::vector<std::string>& round : rounds) {
        std::vector<std::string> found;
        for (const char* extension : {".png", ".dds"}) {
            for (const std::string& path : archive_paths_under(dir, extension)) {
                std::string name = lowered(path.substr(path.find_last_of('/') + 1));
                const size_t dot_at = name.find_last_of('.');
                if (dot_at != std::string::npos) name.resize(dot_at);
                const std::vector<std::string> words = underscore_words(name);
                if (std::find(words.begin(), words.end(), set_word) == words.end()) continue;
                if (std::find(round.begin(), round.end(), words.back()) == round.end()) continue;
                found.push_back(path);
            }
        }
        if (found.size() == 1) return found.front();
    }
    return {};
}

/// Point a freshly built entity at the *current* material of each of its names.
///
/// **An `Ogre::Mesh` is a cached resource and its submeshes cache a
/// `MaterialPtr`, not just a name.** `SubMesh::mMaterial` is resolved when the
/// mesh is first loaded, and `Entity::_initialise()` copies it straight into
/// each `SubEntity` (`OgreEntity.cpp`, "if (subMesh->getMaterial())"). So an
/// entity built from a mesh that was loaded earlier inherits whatever
/// `Material` object was current *then* - and if some script has redeclared
/// that name since, the object is one the manager has already dropped.
///
/// That is fatal rather than untidy, because the RTShaderSystem keys its
/// generated techniques by material *name* and can hold one per name: the
/// dropped object gets no shader technique, falls back to fixed-function, and
/// neither of this port's render systems has one. It is what killed the
/// character-creation screen on the fourth swap - Destroyer, Vanquisher,
/// Alchemist, Vanquisher - where the Vanquisher's hair was rebuilt from a
/// cached mesh whose five `Material_#1/<slot>` pointers dated from the swap
/// before the Alchemist's script replaced them all.
///
/// Cheap, and it must run on *every* new entity: this is a property of mesh
/// caching, not of the wardrobe. repoint_materials() beside it is the other
/// half - it fixes entities that already existed when a name was redefined.
size_t rebind_materials(Ogre::Entity* entity) {
    if (entity == nullptr) return 0;
    auto& materials = Ogre::MaterialManager::getSingleton();
    size_t rebound = 0;
    for (unsigned i = 0; i < entity->getNumSubEntities(); ++i) {
        Ogre::SubEntity* sub = entity->getSubEntity(i);
        const Ogre::MaterialPtr held = sub->getMaterial();
        if (!held) continue;
        const Ogre::MaterialPtr current = materials.getByName(held->getName(), held->getGroup());
        if (current && current.get() != held.get()) {
            sub->setMaterial(current);
            ++rebound;
        }
    }
    unlight_full_emissive(entity);
    return rebound;
}

// `CGenericModel::reInitialize`, for
// each record: technique 0, pass 0 of its material - when that pass has a
// texture unit and its emissive red, blue and green are each exactly 1 - has
// its lighting turned off (`Pass +0xcc` = 0). The material is the shared one,
// so every user of it draws unlit from then on. The console's captured shaders
// for such passes (the crypt's `candle_additive_nolight` and `smoke`, the town
// menu's `billboard_town_mat`) do no lighting: a constant colour of 1 where the
// port, lit, reached 1.1-1.7.
void unlight_full_emissive(Ogre::Entity* entity) {
    if (entity == nullptr) return;
    for (unsigned i = 0; i < entity->getNumSubEntities(); ++i) {
        const Ogre::MaterialPtr& held = entity->getSubEntity(i)->getMaterial();
        if (!held) continue;
        Ogre::Technique* technique = nullptr;
        for (Ogre::Technique* t : held->getTechniques()) {
            if (t->getSchemeName() == Ogre::MSN_DEFAULT) {
                technique = t;
                break;
            }
        }
        if (technique == nullptr || technique->getNumPasses() == 0) continue;
        Ogre::Pass* pass = technique->getPass(0);
        const Ogre::ColourValue& emissive = pass->getSelfIllumination();
        if (pass->getNumTextureUnitStates() == 0 || !pass->getLightingEnabled() ||
            !(emissive.r == 1.0f && emissive.b == 1.0f && emissive.g == 1.0f)) {
            continue;
        }
        pass->setLightingEnabled(false);
        if (auto* shaders = Ogre::RTShader::ShaderGenerator::getSingletonPtr()) {
            shaders->removeAllShaderBasedTechniques(held->getName(), held->getGroup());
        }
    }
}

/// Give a pass the original's rim-light texture unit, or return the one it
/// already has (`CGenericModel::setRimLighting`).
///
/// Appended last, which is where `setHighlightTexture`
/// looks for it: it takes
/// `getTextureUnitState(count - 1)` and swaps its cube map. That is the whole
/// mechanism - the unit is always present, and *which cube map it samples* is
/// the difference between a character standing there and the one the player has
/// selected.
/// The name `setRimLighting` gives a pass it has lit.
const char* const kRimLitPass = "RIMLIT";

Ogre::TextureUnitState* rim_unit(Ogre::Pass* pass, const std::string& texture) {
    if (pass == nullptr) return nullptr;
    Ogre::TextureUnitState* unit = nullptr;
    if (pass->getNumTextureUnitStates() > 1) {
        unit = pass->getTextureUnitState(pass->getNumTextureUnitStates() - 1);
        if (unit->getTextureName() == texture) return unit;
    }
    if (unit == nullptr || unit->getTextureType() != Ogre::TEX_TYPE_CUBE_MAP) {
        unit = pass->createTextureUnitState();
    }
    unit->setTextureName(texture, Ogre::TEX_TYPE_CUBE_MAP);
    unit->setTextureCoordSet(1);
    unit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
   …31401 tokens truncated…lse {
        read.opaque = census.kind != tl::formats::DdsAlpha::Varies;
        read.clear_share = census.clear_share();
    }
    return texture_alphas.emplace(name, read).first->second;
}

// CLevel::bake, its room pieces: every
// `Room Piece` of the level that is bakeable (`BAKE`, the piece's +0x14f,
// true unless the layout says otherwise), shown (+0x6d) and has a model goes
// into one of three `StaticGeometry` sets and its own model is hidden:
//
//   * `stgeompre_` on queue 0x30, a `PRERENDER` piece (+0x14e);
//   * `stgeompost_` on 0x59, a `POSTRENDER` one (+0x14d);
//   * `stgeom_` on 0x31, every other.
//
// The console sets nothing else on them: OGRE's default region (1000 units, so
// a level is a region or a few, one batch per material in each) and no
// rendering distance. The Linux build's `setRegionDimensions` (the level's
// template, 50 by default) and `setRenderingDistance` (48, or 30 in netbook
// mode) are the PC's alone. Nor does it ask whether the piece is animated or
// blends (Linux's `getRoomPieceIsAnimated` test is not on the console): a
// blended piece is batched like the rest and sorts with its region.
//
// The level's `Light`s go to its fourth set, `stgeomLight_` on 0x5f, that
// many times over; place_mesh() keeps those as copies on the light map's group
// instead (see `MeshPlacement::light_density`).
//
// The baked pieces keep their entities, hidden: ground_under() and
// apply_scenery_fill() read them, and StaticGeometry takes the materials the
// sub-entities carry, so every pass edit made to them still applies.
void Renderer::Impl::bake_static_scenery() {
    if (!scene || static_scenery != nullptr) return;

    struct Set {
        const char* name;
        unsigned queue;
        Ogre::StaticGeometry* geometry = nullptr;
        size_t pieces = 0;
    };
    Set sets[3] = {{"stgeompre_", 0x30u}, {"stgeom_", 0x31u}, {"stgeompost_", runic_queue(0x59)}};
    for (Set& set : sets) {
        set.geometry =
            scene->createStaticGeometry(std::string(set.name) + std::to_string(++baked_sets));
        set.geometry->setRenderQueueGroup(static_cast<Ogre::uint8>(set.queue));
    }

    size_t triangles = 0;
    for (const BakeablePiece& piece : bakeable_room_pieces) {
        Ogre::Entity* entity = piece.entity;
        Ogre::SceneNode* node = entity->getParentSceneNode();
        if (node == nullptr || !entity->getVisible()) continue;
        Set& set = sets[piece.render_phase < 0 ? 0 : piece.render_phase > 0 ? 2 : 1];
        set.geometry->addEntity(entity, node->_getDerivedPosition(), node->_getDerivedOrientation(),
                                node->_getDerivedScale());
        ++set.pieces;
        for (const Ogre::SubMesh* sub : entity->getMesh()->getSubMeshes()) {
            if (sub->indexData) triangles += sub->indexData->indexCount / 3;
        }
        entity->setVisible(false);
        // PORT: and out of the scene graph. Hidden, it was still visited by every
        // camera every frame (OGRE's default scene manager tests each node against
        // the frustum, then each object): on the console's Mines floor that was a
        // third of the game's thread, for nearly 4,000 pieces the batch draws
        // anyway. The node keeps its transform and the entity, which is all
        // ground_under() and apply_scenery_fill() read.
        if (Ogre::SceneNode* parent = node->getParentSceneNode()) parent->removeChild(node);
    }
    size_t regions = 0;
    for (Set& set : sets) {
        set.geometry->build();
        regions += set.geometry->getRegions().size();
    }
    static_scenery = sets[1].geometry;

    TL_LOG_INFO(
        "static scenery: %zu / %zu / %zu room piece(s) baked into stgeompre_ / stgeom_ / "
        "stgeompost_, %zu region(s), %zu triangles; %zu bakeable of %zu room pieces",
        sets[0].pieces, sets[1].pieces, sets[2].pieces, regions, triangles,
        bakeable_room_pieces.size(), room_piece_meshes.size());
}

/// Link an animation onto something the *layout* placed, and start it running.
///
/// Torchlight keeps one `.SKELETON` per animation beside the mesh, and a layout
/// node names the one it wants by stem: a `Monster` says `ANIMATION: Idle` and
/// `Idle.SKELETON` sits next to `Ratman.MESH`. That is the same arrangement
/// Renderer::link_animation() handles for the subject, and this is that
/// generalised from "the subject" to any entity - which is the second half of
/// what the town needed. The first half is advance() stepping it.
///
/// **`bind` is not an animation and must not be looked for.** 205 of the
/// archive's monsters author it and there is no `bind.SKELETON` anywhere; it
/// names the mesh's own bind pose, which is what an entity already shows. It is
/// not, however, an instruction to stand still - see the note on the idle
/// fallback in place_layout(), which is what makes the town's merchants breathe.
///
/// The state is found by *difference* rather than by name. A skeleton file names
/// its animation internally and there is no rule that the name matches the
/// filename - `good_zealot/idle.SKELETON` and `Sage/Special_Focus_Cast.SKELETON`
/// are not obliged to agree with their stems, and a name lookup that missed
/// would fail silently as a character that simply never moves. Taking whatever
/// state the link *added* cannot miss.
/// The first of `wanted` that names a state on this set, matched without case.
///
/// The two spellings do not have to agree and in the town they do not:
/// `RalinAnimation` asks for `idle` while `Idle.SKELETON` names its animation
/// `Idle`. And a caller asks for an animation *class* - `Attack`, `Die` - which
/// archive_animation_in_dir() resolves to whichever numbered or suffixed file
/// the mesh ships, so the Ratman's `Attack` arrives as `Attack1`; the source
/// file's own animation names are therefore candidates alongside the requested
/// one.
// PORT: an OGRE animation state found by any of several names.
Ogre::AnimationState* state_named_any_of(Ogre::AnimationStateSet* states,
                                         const std::vector<std::string>& wanted) {
    if (states == nullptr) return nullptr;
    for (const std::string& want : wanted) {
        for (const auto& entry : states->getAnimationStates()) {
            if (entry.first.size() != want.size()) continue;
            if (std::equal(want.begin(), want.end(), entry.first.begin(), [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) ==
                           std::tolower(static_cast<unsigned char>(b));
                })) {
                return entry.second;
            }
        }
    }
    return nullptr;
}

unsigned Renderer::Impl::entity_track(EntityAnimation& model, Ogre::AnimationState* state) {
    for (size_t i = 0; i < model.states.size(); ++i) {
        if (model.states[i] == state) return static_cast<unsigned>(i);
    }
    const unsigned track =
        model.animator.add_animation(state->getAnimationName(), state->getLength());
    model.states.push_back(state);
    tl::core::ModelTrack seen;
    seen.length = state->getLength();
    seen.time = state->getTimePosition();
    seen.weight = state->getWeight();
    seen.enabled = state->getEnabled();
    seen.loop = state->getLoop();
    model.written.push_back(seen);
    // The animator starts from the state as it stands.
    model.animator.tracks()[track] = seen;
    return track;
}

void Renderer::Impl::write_entity_animation(EntityAnimation& model) {
    for (size_t i = 0; i < model.states.size(); ++i) {
        const tl::core::ModelTrack& now = model.animator.tracks()[i];
        tl::core::ModelTrack& was = model.written[i];
        Ogre::AnimationState* state = model.states[i];
        // Never loop something with no duration: `setTimePosition` wraps a
        // looping state with `fmod(t, length)`, and that is NaN for 0.
        if (now.loop != was.loop) state->setLoop(now.loop && state->getLength() > 0.0f);
        if (now.time != was.time) state->setTimePosition(now.time);
        if (now.weight != was.weight) state->setWeight(now.weight);
        if (now.enabled != was.enabled) state->setEnabled(now.enabled);
        was = now;
    }
}

bool Renderer::Impl::play_scene_animation(Ogre::Entity* entity, const std::string& mesh_path,
                                          const std::string& animation, bool loops, float rate,
                                          bool optional, Ogre::AnimationState** started, bool armed,
                                          std::string* played, float blend, bool queue) {
    if (!entity || names_the_bind_pose(animation)) return false;
    if (played != nullptr) *played = animation;

    if (!entity->hasSkeleton()) {
        if (!optional) {
            TL_LOG_WARN("  '%s' asks for animation '%s' but carries no skeleton", mesh_path.c_str(),
                        animation.c_str());
        }
        return false;
    }

    const std::string dir = dir_of_asset(mesh_path);
    std::string path = archive_path_in_dir(animation + ".SKELETON", dir);
    if (path.empty()) {
        // Not under that exact name. A layout, and this engine, name an
        // animation by its *state* - `Idle`, `Die`, `Run` - and a mesh is free
        // to ship it as `Idle_Armed` or `Die_Armed2`; see
        // archive_animation_in_dir(). Without this the skeleton warriors stood
        // in their bind pose and vanished instead of falling over, because
        // every one of their files carries the suffix.
        path = archive_animation_in_dir(dir, animation, armed, 0);
        if (!path.empty()) {
            TL_LOG_INFO("  '%s' plays '%s' as '%s'", mesh_path.c_str(), animation.c_str(),
                        path.c_str());
        }
    }
    // **A rig that has no RUN walks, and one that has no WALK runs**, and the
    // fallback is symmetric and explicit in the original.
    // `CCharacter::updateAnimation`
    // picks the locomotion animation twice over, once for each gait: wanting
    // RUN it calls `CGenericModel::animationExists("RUN")` and,
    // when that is false, jumps to the WALK block; wanting WALK it
    // calls `animationExists("WALK")` and, when *that* is false,
    // falls through to the RUN block.
    //
    // Seven model directories in the archive ship one gait and not the other -
    // `ratmanforeman`, the four elemental wisps, `wisp` and `invisible` - and
    // fourteen ship RUN with no WALK, the player classes among them. The
    // Ratlin Foreman is what this was found on: the hunt AI asks for `Run`,
    // `media/models/RatmanForeman/` holds only `Walk.SKELETON`, the request
    // failed quietly (it is `optional`), and the unit therefore kept whatever
    // it was playing - which, once it had swung at anything, was a *looping*
    // `Attack1`. It walked the level swinging a hammer at nothing, which is
    // exactly what the log shows: 12 `play 'Attack'`, 1 blow landed, and 15
    // `play 'Run' on 0 Ratlin Foreman entities` between them.
    //
    // Its `WALKINGSPEED` and `RUNNINGSPEED` are both authored 1.0, so the
    // substitution is not a compromise: one gait is all that unit has.
    std::string resolved = animation;
    if (path.empty()) {
        const std::string want = lowered(animation);
        const char* other = want == "run" ? "Walk" : (want == "walk" ? "Run" : nullptr);
        if (other != nullptr) {
            path = archive_path_in_dir(std::string(other) + ".SKELETON", dir);
            if (path.empty()) path = archive_animation_in_dir(dir, other, armed, 0);
            if (!path.empty()) {
                resolved = other;
                if (played != nullptr) *played = resolved;
                TL_LOG_INFO("  '%s' has no '%s'; plays '%s' as '%s'", mesh_path.c_str(),
                            animation.c_str(), other, path.c_str());
            }
        }
    }
    if (path.empty()) {
        // `optional` is the engine's default idle rather than something the
        // layout asked for by name, so a mesh that ships none is an ordinary
        // fact about that mesh and not a fault worth a warning.
        if (!optional) {
            TL_LOG_WARN("  no '%s.SKELETON' beside '%s'", animation.c_str(), mesh_path.c_str());
        }
        return false;
    }

    // A pose rather than an animation, which the archive does not distinguish by
    // name or extension - only by whether the file holds any track. Applied
    // directly and not stepped: there is nothing to advance.
    Ogre::SkeletonPtr source;
    try {
        // SkeletonManager inherits ResourceManager::load(), which is declared to
        // return a ResourcePtr; the cast is what every OGRE caller does here.
        source = Ogre::static_pointer_cast<Ogre::Skeleton>(
            Ogre::SkeletonManager::getSingleton().load(path, kGameGroup));
    } catch (const Ogre::Exception& e) {
        TL_LOG_WARN("  could not load '%s': %s", path.c_str(), e.getDescription().c_str());
        return false;
    }
    bool timed_pose = false;
    if (queue && !skeleton_has_tracks(source)) return false;
    if (!skeleton_has_tracks(source)) {
        // Whatever the entity was playing stops here. A state left enabled
        // keeps applying its keys to bones the pose now holds manually, which
        // `Skeleton::reset()` skips - each frame's keys land on the last
        // frame's result, and a mimic posed `Asleep` over its looping `Idle`
        // sat there turning its lid.
        if (Ogre::AnimationStateSet* playing = entity->getAllAnimationStates()) {
            for (const auto& entry : playing->getAnimationStates()) {
                if (entry.second->getEnabled()) entry.second->setEnabled(false);
            }
        }
        // And the animator lets go of them (`clearAnimations`), so the next
        // frame does not step them on again.
        if (const auto held = entity_animations.find(entity); held != entity_animations.end()) {
            held->second.animator.clear();
            for (size_t i = 0; i < held->second.written.size(); ++i) {
                held->second.written[i] = held->second.animator.tracks()[i];
            }
        }
        const size_t posed = pose_from_skeleton(entity, source);
        if (posed == 0) {
            TL_LOG_WARN("  '%s' holds no animation track and no bone the mesh shares",
                        path.c_str());
            return false;
        }
        TL_LOG_INFO(
            "  posing %s from '%s' (%zu bone(s); the file holds a pose, not an "
            "animation)",
            mesh_path.c_str(), animation.c_str(), posed);
        // A pose with a length still runs its clock: the elemental wisps'
        // `die` is 1.27 s over no track, and its `.ANIMATION` keys - the
        // impact burst at frame 0, `FADEOUTMESH` - fire on that time as on any
        // other animation's (`CCharacter::updateAnimation`
        // reads the keys against the playing
        // animation's time). `Dead1` is 0 s long and stays a pose alone.
        float length = 0.0f;
        for (unsigned short i = 0; i < source->getNumAnimations(); ++i) {
            length = std::max(length, source->getAnimation(i)->getLength());
        }
        if (!(length > 0.0f)) return true;
        timed_pose = true;
    }

    // A pose an earlier file left (`pose_from_skeleton`) holds its bones
    // manually, and `Skeleton::reset()` skips a manually controlled bone - so an
    // animation played over it adds each frame's keys onto the last frame's
    // result instead of the rest pose, and a chest's lid, posed `closed` and then
    // played `opening`, kept turning. The bones go back to the animation.
    if (entity->hasSkeleton() && !timed_pose) {
        for (Ogre::Bone* bone : entity->getSkeleton()->getBones()) {
            if (!bone->isManuallyControlled()) continue;
            bone->setManuallyControlled(false);
            bone->reset();
        }
    }

    std::set<std::string> before;
    if (const Ogre::AnimationStateSet* states = entity->getAllAnimationStates()) {
        for (const auto& entry : states->getAnimationStates()) before.insert(entry.first);
    }

    if (!link_path_onto(entity, path)) return false;

    Ogre::AnimationStateSet* states = entity->getAllAnimationStates();
    if (!states) return false;

    // Which state this file actually brought in.
    //
    // **By name first, and only then by what appeared.** The set difference
    // below used to come first, on the reasoning that the first copy of a
    // monster is the one that links the file and so the one new state must be
    // it. That is true only while one entity at a time is linking: OGRE keeps
    // linked-skeleton sources on the **shared master skeleton resource**, not
    // on the instance (see the note in the fallback), so an animation linked by
    // *another* entity of the same mesh turns up in this entity's state set the
    // next time it is refreshed - and then it is "new" here too, and the
    // difference picks it.
    //
    // Three Varkolyn coming out of one chute is exactly that case, and it is
    // what the shaft ambush produced: `Varkolyn_Chuteb` links
    // `Mine_ChuteA_Spawn2`, and a moment later the entity that had linked
    // `Mine_ChuteA_Spawn1` asks for `Idle`, resolves the path to
    // `Idle.SKELETON` correctly, and is handed `Mine_ChuteA_Spawn1` because
    // that is what was new in its set. The unit then played its entrance on a
    // loop instead of going back to standing.
    //
    // The candidate list is the requested name plus the source file's own
    // animation names, which is what `_mergeSkeletonAnimations()` merges under
    // - so it names the right state whoever linked it and in whatever order.
    std::vector<std::string> wanted_names{resolved};
    for (unsigned short i = 0; i < source->getNumAnimations(); ++i) {
        wanted_names.push_back(source->getAnimation(i)->getName());
    }
    Ogre::AnimationState* added = state_named_any_of(states, wanted_names);
    if (!added) {
        for (const auto& entry : states->getAnimationStates()) {
            if (before.count(entry.first) == 0) {
                added = entry.second;
                break;
            }
        }
    }
    if (!added) {
        // Already linked, which is the ordinary case for the second and later
        // copies of a monster: the town places four Varkolyn corpses from one
        // mesh and one animation, and OGRE's linked-skeleton sources live on the
        // shared master skeleton resource rather than on the instance, so the
        // second entity's link adds nothing new and its state is simply already
        // there.
        //
        // Matched case-insensitively, because the two spellings do not have to
        // agree and in the town they do not: `RalinAnimation` asks for `idle`
        // while `Idle.SKELETON` names its animation `Idle`, so an exact lookup
        // left two of the three Ratlins standing still while the first - which
        // took the "newly added" path and never consulted the name - moved.
        //
        // Matched against **the source file's own animation names** as well as
        // the requested one, because the two are routinely different. A caller
        // asks for an animation *class* - `Attack`, `Die`, `Idle` -
        // and archive_animation_in_dir() resolves that to whichever numbered or
        // suffixed file the mesh actually ships; `_mergeSkeletonAnimations()`
        // then copies the animations in under the names the *file* gives them.
        // So the Ratman's `Attack` resolves to `Attack1.SKELETON`, which merges
        // an animation called `Attack1`, and looking only for `Attack` finds
        // nothing. The first copy of the monster was unaffected - it takes the
        // "newly added" path above and never consults a name - so every Ratman
        // after the first stood still through the fight.
        // Asked and answered above: state_named_any_of() runs over exactly
        // these candidates before the set difference is consulted at all.
    }
    if (!added) {
        TL_LOG_WARN("  linked '%s' onto '%s' but it added no animation state", path.c_str(),
                    mesh_path.c_str());
        return false;
    }

    // The entity's `CGenericModel` animation half (core/model_animation.h):
    // `playAnimation` replaces whatever played, `blendAnimation` fades it in
    // over `blend` seconds while the old one fades out under it, at the
    // weights `updateAnimation` works out each frame. States switched on by a
    // path that does not go through it (a pose, the bestiary) are not its to
    // fade, and go off as before.
    EntityAnimation& model = entity_animations[entity];
    if (queue) {
        // Behind what plays; nothing is switched off and the rate of what
        // plays now stays as it is.
        const unsigned track = entity_track(model, added);
        model.animator.queue_blend(static_cast<int>(track), loops, blend, rate);
        write_entity_animation(model);
        if (started != nullptr) *started = added;
        TL_LOG_INFO("  queueing '%s' on %s behind what plays (blend %.2fs)",
                    added->getAnimationName().c_str(), mesh_path.c_str(),
                    static_cast<double>(blend));
        return true;
    }
    for (const auto& entry : states->getAnimationStates()) {
        if (entry.second != added && entry.second->getEnabled() &&
            std::find(model.states.begin(), model.states.end(), entry.second) ==
                model.states.end()) {
            entry.second->setEnabled(false);
        }
    }
    const unsigned track = entity_track(model, added);
    if (blend > 0.0f) {
        model.animator.blend(track, loops, blend, rate);
    } else {
        model.animator.play(track, loops, rate);
    }
    // What the state really is now, so the one started goes on even where
    // another path switched it off since it was last written.
    model.written[track].enabled = added->getEnabled();
    model.written[track].time = added->getTimePosition();
    // At the record's time from now, not from the next update: the callers time
    // the animation's end off the state, and a state replayed from its end
    // would read as already over.
    model.animator.tracks()[track].set_time(model.animator.active().front().time);
    write_entity_animation(model);
    if (started != nullptr) *started = added;

    // One entry per entity, replaced rather than appended. advance() steps every
    // enabled state of every entry, so an entity listed twice has its animation
    // advanced twice per frame - and the script starts and stops animations on
    // the same unit repeatedly through a cutscene, so the seer's collapse ran at
    // several times speed by the time the scene reached it.
    auto existing = std::find_if(scene_animations.begin(), scene_animations.end(),
                                 [entity](const SceneAnimation& a) { return a.entity == entity; });
    if (existing != scene_animations.end()) {
        existing->rate = rate;
    } else {
        scene_animations.push_back(SceneAnimation{entity, rate});
    }
    TL_LOG_INFO("  animating %s with '%s' (%.2fs%s, rate %.2f)", mesh_path.c_str(),
                added->getAnimationName().c_str(), static_cast<double>(added->getLength()),
                loops ? ", looping" : ", once", static_cast<double>(rate));
    return true;
}

namespace renderer_detail {
namespace {
bool g_caching_particles = false;
}  // namespace

bool caching_particles() { return g_caching_particles; }

// `CLayout::setCacheingParticlesForLevel`: `g_bCachParticles = 1`,
// whatever it is passed.
void set_caching_particles() { g_caching_particles = true; }

/// Start a placed system over: `CParticle::Start`,
/// its clock back to zero.
void restart_particle_system(ParticleSystem& system) {
    system.elapsed = 0.0f;
    system.finished = false;
    system.pu_dying = false;
    start_torchlight_particle(system.pu);
}

/// `CParticle::Stop(false)`: every emitter
/// stops releasing now; what is alive lives out its life, then the system
/// stops.
void stop_particle_emission(ParticleSystem& system) {
    if (system.finished) return;
    stop_torchlight_particle_emission(system.pu);
    system.pu_dying = true;
}

}  // namespace renderer_detail
/// Place one `Particle` node: `CParticleWrapper::createParticleSystem`
/// through the glue, drawn by Torchlight's
/// ParticleUniverse fork.
bool Renderer::Impl::place_particle(const tl::formats::CmpScene& layout,
                                    const tl::formats::CmpNode& node, const Ogre::Vector3& position,
                                    const Ogre::Quaternion& orientation) {
    using namespace tl::formats;

    if (const CmpField* visible = node.field(cmp_field::particle::kVisible)) {
        if (visible->count() == 1 && !visible->as_bool()) return false;
    }

    if (!particle_universe.running() || !particle_universe_takes(placing_layout_path)) return false;
    std::string missing;
    if (!can_build_torchlight_particle(layout, node, &missing)) {
        TL_LOG_WARN("  particle '%s': %s is not in the ParticleUniverse fork; not drawn",
                    node.name.c_str(), missing.c_str());
        return false;
    }
    return place_torchlight_particle(layout, node, position, orientation);
}

void Renderer::Impl::arm_particle_timer(size_t first, size_t last) {
    float timeout = 0.0f;
    float window = 0.0f;
    bool never = false;
    for (size_t i = first; i < last && i < particle_systems.size(); ++i) {
        const ParticleSystem& system = particle_systems[i];
        if (system.timer_armed) continue;
        timeout = std::max(timeout, system.fixed_timeout);
        window = std::max(window, system.finite_window);
        never = never || system.non_expiring;
    }
    float stop_at = timeout > 0.0f ? timeout : window;
    if (never) stop_at = 0.0f;
    if (stop_at > 0.0f) stop_at += 0.1f;
    for (size_t i = first; i < last && i < particle_systems.size(); ++i) {
        if (particle_systems[i].timer_armed) continue;
        particle_systems[i].stop_at = stop_at;
        particle_systems[i].timer_armed = true;
    }
}

bool Renderer::Impl::place_torchlight_particle(const tl::formats::CmpScene& layout,
                                               const tl::formats::CmpNode& node,
                                               const Ogre::Vector3& position,
                                               const Ogre::Quaternion& orientation) {
    ParticleBuildContext context;
    context.scene = scene;
    context.camera = camera;
    context.resource_group = kGameGroup;
    context.resolve_texture = [](const std::string& reference) {
        return renderer_detail::resolve_archive_path(reference);
    };
    // The entity renderer's mesh, loaded as a layout's is, its own material
    // script first (a particle layout places no meshes, so place_layout() has
    // parsed none for it).
    context.load_mesh = [](const std::string& reference) {
        const std::string path = renderer_detail::resolve_archive_path(reference);
        if (path.empty()) {
            TL_LOG_WARN("particle mesh '%s' is not in the archive", reference.c_str());
            return std::string();
        }
        parse_sibling_material(path);
        const Ogre::MeshPtr mesh = load_layout_mesh(path);
        return mesh ? mesh->getName() : std::string();
    };
    // The original's queues are 90 + `SORT INDEX` (+ 5 for a light); this port
    // draws particles from `kParticleGroupBase` with the same offsets, after the
    // characters' group and below the overlay ("IS LIGHT").
    context.map_render_queue = [](unsigned queue) {
        const unsigned offset = queue >= 0x5a ? queue - 0x5a : 0u;
        return std::min<unsigned>(kParticleGroupBase + offset, 90);
    };

    ParticleSystem system;
    if (!build_torchlight_particle(context, layout, node, position, orientation, system.pu)) {
        return false;
    }
    // An `IS LIGHT` particle is drawn into the light map, and the view refuses
    // its groups (`CGame::renderableQueued`, ViewRenderables). The Xbox
    // build does this: a captured town frame draws the lanterns' `candleglow`
    // discs, a `glowcyan` and a sparkle
    // into the light map (target 208) and none of them in the view.
    // How the console's light-map pass admits them is not
    // settled: on Linux it sets the scene's mask to 2 (`CGameClient::
    // preRenderTargetUpdate`), which a system with the default flag 1
    // (`CGame::chooseSceneManager`, stores it too) would fail.
    if (system.pu.is_light && system.pu.movable != nullptr) {
        system.pu.movable->addVisibilityFlags(kVisibilityLightMap);
    }
    system.owner = particle_owner;
    system.node = node.id;
    system.node_owner = node_sink != nullptr ? node_sink : &scripted_nodes;
    system.wrapper = placing_under_link && !placing_particle_file;
    system.fixed_timeout = system.pu.fixed_timeout;
    system.finite_window = system.pu.finite_window;
    system.non_expiring = system.pu.non_expiring;
    // `CParticle::Start`: a placed particle
    // plays from the start. A wrapper waits for its node's `Start Particle`.
    if (!system.wrapper) start_torchlight_particle(system.pu);
    TL_LOG_INFO("  particle '%s': ParticleUniverse (Torchlight's fork) at (%.2f, %.2f, %.2f)%s%s",
                node.name.c_str(), static_cast<double>(position.x), static_cast<double>(position.y),
                static_cast<double>(position.z), system.pu.non_expiring ? ", does not expire" : "",
                system.wrapper ? ", a wrapper of its layout: not started" : "");
    particle_systems.push_back(std::move(system));
    particle_index_add(particle_systems.size() - 1);
    return true;
}

void Renderer::Impl::step_torchlight_particle(ParticleSystem& system, float delta) {
    // `CParticle::updateLevelObject`: only
    // while the layout's timer runs or its particles are dying out. When the
    // timer is due - at once for a particle stopped without one - and nothing
    // is left alive of a dying one, `Stop(true)`.
    if (system.finished) return;
    const bool timed = system.stop_at > 0.0f;
    if (!timed && !system.pu_dying) return;
    system.elapsed += delta;
    const bool due = !timed || system.elapsed >= system.stop_at;
    if (due && (!system.pu_dying || !torchlight_particle_updating(system.pu))) {
        stop_torchlight_particle(system.pu);
        system.pu_dying = false;
        system.finished = true;
    }
}

void Renderer::Impl::step_particles(float delta) {
    forget_dead_cparticles();
    if (particle_systems.empty()) return;

    for (ParticleSystem& system : particle_systems) {
        if (system.pu.system == nullptr) continue;
        if (system.wrapper) {
            const bool was_playing = system.pu.playing;
            update_torchlight_wrapper(system.pu);
            if (was_playing && !system.pu.playing) {
                TL_LOG_INFO("script: particle wrapper %lld stops, its last particle gone",
                            static_cast<long long>(system.node));
            }
        } else {
            step_torchlight_particle(system, delta);
        }
    }
}

namespace renderer_detail {
Ogre::Vector3 default_camera_offset(float camera_mult) {
    const float distance = kDefaultCameraDistance * camera_mult;
    return Ogre::Vector3(0.5f * distance, 0.75f * distance, -0.5f * distance);
}

void for_each_triangle(
    const Ogre::MeshPtr& mesh, const Ogre::Matrix4& to_world,
    const std::function<void(const Ogre::Vector3&, const Ogre::Vector3&, const Ogre::Vector3&)>& fn,
    const std::function<bool(const std::string&)>& accept_material) {
    for (const Ogre::SubMesh* sub : mesh->getSubMeshes()) {
        if (accept_material && !accept_material(sub->getMaterialName())) continue;

        const Ogre::VertexData* vertex_data =
            sub->useSharedVertices ? mesh->sharedVertexData : sub->vertexData;
        if (!vertex_data || !sub->indexData) continue;

        const Ogre::VertexElement* pos_elem =
            vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        if (!pos_elem) continue;

        Ogre::HardwareVertexBufferSharedPtr vbuf =
            vertex_data->vertexBufferBinding->getBuffer(pos_elem->getSource());
        Ogre::HardwareIndexBufferSharedPtr ibuf = sub->indexData->indexBuffer;
        if (!vbuf || !ibuf) continue;

        std::vector<Ogre::Vector3> points(vertex_data->vertexCount);
        {
            Ogre::HardwareBufferLockGuard lock(vbuf, Ogre::HardwareBuffer::HBL_READ_ONLY);
            const unsigned char* base = static_cast<const unsigned char*>(lock.pData);
            for (size_t v = 0; v < vertex_data->vertexCount; ++v) {
                const float* p = nullptr;
                pos_elem->baseVertexPointerToElement(
                    const_cast<unsigned char*>(base + v * vbuf->getVertexSize()), &p);
                points[v] = to_world * Ogre::Vector3(p[0], p[1], p[2]);
            }
        }

        Ogre::HardwareBufferLockGuard lock(ibuf, Ogre::HardwareBuffer::HBL_READ_ONLY);
        const bool wide = ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT;
        const uint32_t* i32 = static_cast<const uint32_t*>(lock.pData);
        const uint16_t* i16 = static_cast<const uint16_t*>(lock.pData);
        const size_t triangles = sub->indexData->indexCount / 3;
        for (size_t t = 0; t < triangles; ++t) {
            const size_t a = wide ? i32[t * 3 + 0] : i16[t * 3 + 0];
            const size_t b = wide ? i32[t * 3 + 1] : i16[t * 3 + 1];
            const size_t c = wide ? i32[t * 3 + 2] : i16[t * 3 + 2];
            if (a >= points.size() || b >= points.size() || c >= points.size()) continue;
            fn(points[a], points[b], points[c]);
        }
    }
}

}  // namespace renderer_detail
bool Renderer::Impl::ground_under(const Ogre::Vector3& from, float& surface_y) const {
    // Straight down, against everything the layout placed. Both faces are
    // tested: a painted terrain billboard is a one-sided sheet and its winding
    // is whatever the exporter chose, so rejecting back faces loses hits.
    const Ogre::Ray ray(from, Ogre::Vector3::NEGATIVE_UNIT_Y);
    bool hit = false;
    Ogre::Real nearest = 0.0f;

    for (Ogre::Entity* entity : layout_meshes) {
        // Somebody standing there is not the ground under it.
        if (unit_meshes.count(entity) != 0) continue;
        // Derived, not cached: nothing has rendered yet when this runs, so the
        // cached world box is still null and every candidate would be skipped.
        const Ogre::AxisAlignedBox box = entity->getWorldBoundingBox(true);
        if (box.isNull()) continue;
        if (from.x < box.getMinimum().x || from.x > box.getMaximum().x) continue;
        if (from.z < box.getMinimum().z || from.z > box.getMaximum().z) continue;
        if (!entity->getParentSceneNode()) continue;

        const Ogre::Matrix4 to_world = entity->getParentSceneNode()->_getFullTransform();
        for_each_triangle(
            entity->getMesh(), to_world,
            [&](const Ogre::Vector3& a, const Ogre::Vector3& b, const Ogre::Vector3& c) {
                const std::pair<bool, Ogre::Real> r =
                    Ogre::Math::intersects(ray, a, b, c, true, true);
                if (!r.first) return;
                if (!hit || r.second < nearest) {
                    nearest = r.second;
                    hit = true;
                }
            });
    }

    if (hit) surface_y = static_cast<float>(from.y - nearest);
    return hit;
}

void Renderer::Impl::stand_subject_at(const tl::formats::CmpNode& start,
                                      const Ogre::Vector3& origin) {
    if (!subject_node || !start.has_position()) return;

    Ogre::Vector3 where =
        origin + Ogre::Vector3(start.position().x, start.position().y, start.position().z);

    // Player Start's Y is not a ground height. It sits 1.215 in the town menu
    // while the terrain below tops out at 0.5, so taking it literally leaves the
    // character standing in mid-air. Drop him onto whatever the layout actually
    // placed, starting the ray well above him so a hill between the marker and
    // the surface is still found.
    float surface = 0.0f;
    if (ground_under(Ogre::Vector3(where.x, where.y + 20.0f, where.z), surface)) {
        TL_LOG_INFO("  ground at %.3f under Player Start (marker Y %.3f)",
                    static_cast<double>(surface), static_cast<double>(where.y));
        where.y = surface;
    } else {
        TL_LOG_WARN("  no ground under Player Start; using its own Y %.3f",
                    static_cast<double>(where.y));
    }
    subject_node->setPosition(where);

    // Facing: the marker's first vector is the direction it points, not a
    // matrix column to install.
    //
    // Player Start's is (0.891, 0, 0.454) - straight at the camera, which sits
    // out along +X - and Torchlight's character meshes face +Z in their own
    // space. Treating the three vectors as a rotation basis and applying it
    // turned him a quarter-circle and left him facing away; rotating the mesh's
    // +Z onto that vector is what the data is actually saying.
    //
    // Flattened to the horizontal plane deliberately. These markers carry no
    // pitch - their up vector is exactly (0, 1, 0) - and a character leaning out
    // of the ground plane would be wrong however the numbers were read.
    if (start.vec3s.size() >= 2) {
        Ogre::Vector3 facing(start.vec3s[1].x, 0.0f, start.vec3s[1].z);
        if (!facing.isZeroLength()) {
            facing.normalise();
            subject_node->setOrientation(Ogre::Vector3::UNIT_Z.getRotationTo(facing));
            TL_LOG_INFO("  facing (%.3f, 0, %.3f)", facing.x, facing.z);
        }
    }

    TL_LOG_INFO("  subject at Player Start (%.2f, %.2f, %.2f)", where.x, where.y, where.z);
}

void Renderer::Impl::aim_camera(const tl::formats::CmpScene& layout, const Ogre::Vector3& origin) {
    const tl::formats::CmpNode* eye = layout.find("Camera");
    const tl::formats::CmpNode* target = layout.find("Camera Target");
    if (!eye || !target || !eye->has_position() || !target->has_position()) return;

    const Ogre::Vector3 eye_pos(eye->position().x, eye->position().y, eye->position().z);
    const Ogre::Vector3 target_pos(target->position().x, target->position().y,
                                   target->position().z);

    layout_has_camera = true;
    layout_camera_target = origin + target_pos;

    // Reset the orientation first, and pin the yaw axis. Without a fixed yaw
    // axis OGRE's lookAt takes the shortest rotation from the node's *current*
    // orientation - and frame_camera_on() has already aimed this node at the
    // Alchemist - so the second aim accumulates roll. The symptom is a scene
    // tilted about twenty degrees, which reads as wrongly placed geometry
    // rather than as a camera problem, and sent me looking at node rotations.
    camera_node->setOrientation(Ogre::Quaternion::IDENTITY);
    camera_node->setFixedYawAxis(true, Ogre::Vector3::UNIT_Y);
    camera_node->setPosition(origin + eye_pos);
    camera_node->lookAt(origin + target_pos, Ogre::Node::TS_WORLD);

    // The framing is the layout's, so the clip planes have to suit it rather
    // than the subject's bounding sphere that frame_camera_on() sized them for.
    const Ogre::Real distance = (target_pos - eye_pos).length();
    camera->setNearClipDistance(std::max(0.05f, static_cast<float>(distance) * 0.02f));
    camera->setFarClipDistance(std::max(200.0f, static_cast<float>(distance) * 20.0f));

    TL_LOG_INFO("camera from layout: eye (%.2f, %.2f, %.2f) -> target (%.2f, %.2f, %.2f)",
                eye_pos.x, eye_pos.y, eye_pos.z, target_pos.x, target_pos.y, target_pos.z);
}

}  // namespace tl::render
