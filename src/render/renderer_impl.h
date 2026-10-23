Warning: truncated output (original token count: 38777)
Total output lines: 3153

// The shared declarations of the renderer's implementation files (renderer*.cpp).
#pragma once

#include "render/renderer.h"

#include "core/layout_helpers.h"
#include "core/layout_maths.h"
#include "core/saved_orientation.h"
#include "core/world.h"

#include "core/attack_button.h"
#include "core/camera_shake.h"
#include "core/character.h"
#include "core/character_sheet.h"
#include "core/cinematics.h"
#include "core/combine.h"
#include "core/hotbar.h"
#include "core/howto_menu.h"
#include "core/inventory.h"
#include "core/inventory_screen.h"
#include "core/item_drop.h"
#include "core/item_use.h"
#include "core/merchant.h"
#include "core/model_animation.h"
#include "core/stats_page.h"
#include "render/automap.h"
#include "render/cinematic_menu.h"
#include "render/death_screen.h"
#include "render/display_gamma.h"
#include "render/game_hud.h"
#include "render/gl_thread.h"
#include "render/gpu_skinning.h"
#include "render/gpu_timer.h"
#include "render/hud.h"
#include "render/inventory_menu.h"
#include "render/layout_screen.h"
#include "render/menu.h"
#include "render/particle_systems.h"
#include "render/particle_universe.h"
#include "render/particle_wrappers.h"
#include "render/quest_dialog.h"
#include "render/render_system_choice.h"
#include "render/touch_overlay.h"
#include "render/unit_opacity.h"

#include "core/active_range.h"
#include "core/affix_book.h"
#include "core/animation_manifest.h"
#include "core/asset_path.h"
#include "core/audio.h"
#include "core/average_fps.h"
#include "core/breakable.h"
#include "core/character_effects.h"
#include "core/collision.h"
#include "core/context_tips.h"
#include "core/death.h"
#include "core/dungeon.h"
#include "core/enchanting.h"
#include "core/fishing.h"
#include "core/game_speed.h"
#include "core/game_ui.h"
#include "core/gamepad.h"
#include "core/input.h"
#include "core/item_effects.h"
#include "core/item_label.h"
#include "core/item_name.h"
#include "core/item_roll.h"
#include "core/item_save.h"
#include "core/item_sets.h"
#include "core/light_override.h"
#include "core/log.h"
#include "core/missile.h"
#include "core/music_object.h"
#include "core/navigation.h"
#include "core/no_spawn_region.h"
#include "core/passability.h"
#include "core/pet.h"
#include "core/pet_commands.h"
#include "core/platform.h"
#include "core/player.h"
#include "core/populate.h"
#include "core/quest.h"
#include "core/random_groups.h"
#include "core/retirement.h"
#include "core/script.h"
#include "core/settings_menu.h"
#include "core/skill_animation.h"
#include "core/skill_cast.h"
#include "core/skill_text.h"
#include "core/skills.h"
#include "core/skills_tab.h"
#include "core/sound_data.h"
#include "core/spawn_class.h"
#include "core/spell_remove.h"
#include "core/stats.h"
#include "core/targeting.h"
#include "core/text.h"
#include "core/text_event.h"
#include "core/town_portal.h"
#include "core/tray_moves.h"
#include "core/tray_press.h"
#include "core/unit_observers.h"
#include "core/unit_resource.h"
#include "core/unit_themes.h"
#include "core/unit_types.h"
#include "core/wardrobe.h"
#include "core/warp.h"
#include "core/waypoints.h"
#include "core/weapon_procs.h"
#include "core/weapon_trail.h"
#include "core/weapons.h"
#include "formats/adm.h"
#include "formats/cmp.h"
#include "formats/ogg.h"
#include "formats/wav.h"

#include <array>
#include <fstream>

#include <OgreAnimation.h>
#include <OgreAnimationState.h>
#include <OgreAnimationTrack.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <OgreBone.h>
#include <OgreCamera.h>
#include <OgreCodec.h>
#include <OgreControllerManager.h>
#include <OgreDataStream.h>
#include <OgreEntity.h>
#include <OgreFrameListener.h>
#include <OgreFrustum.h>
#include <OgreGpuProgramManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreImage.h>
#include <OgreKeyFrame.h>
#include <OgreLogManager.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreMatrix3.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreMeshSerializer.h>
#include <OgrePass.h>
#include <OgreRectangle2D.h>
#include <OgreRenderQueueListener.h>
#include <OgreRenderTargetListener.h>
#include <OgreRenderTexture.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSkeletonInstance.h>
#include <OgreSkeletonManager.h>
#include <OgreStaticGeometry.h>
#include <OgreSubEntity.h>
#include <OgreSubMesh.h>
#include <OgreTagPoint.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <OgreTextureUnitState.h>
#include <OgreTimer.h>
#include <OgreViewport.h>

#include <OgreShaderGenerator.h>

// Static plugin registration: no plugins.cfg and no dlopen (the Switch has no dynamic
// loader). Only the render system differs: the console's OGRE has GLES2 alone, unless
// built with TL_SWITCH_GL3PLUS (render/render_system_choice.h).
#if defined(TL_PLATFORM_SWITCH) || defined(TL_PLATFORM_ANDROID) || defined(TL_PLATFORM_IOS)
#include <OgreGLES2Plugin.h>
#if defined(TL_SWITCH_GL3PLUS)
#include <OgreGL3PlusPlugin.h>
#endif
#else
#include <OgreGL3PlusPlugin.h>
#include <OgreGLPlugin.h>
#endif
#include <OgreSTBICodec.h>

#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <initializer_list>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef TL_OGRE_MEDIA_DIR
#define TL_OGRE_MEDIA_DIR ""
#endif

namespace tl::render {
namespace renderer_detail {}
using namespace renderer_detail;
namespace ogre = tl::core::ogre;

// PORT: the game's maths (core/ogre_types.h) and OGRE's, one to the other.
inline Ogre::Vector3 to_ogre(const ogre::Vector3& v) { return Ogre::Vector3(v.x, v.y, v.z); }
inline Ogre::Quaternion to_ogre(const ogre::Quaternion& q) {
    return Ogre::Quaternion(q.w, q.x, q.y, q.z);
}
inline ogre::Vector3 from_ogre(const Ogre::Vector3& v) { return ogre::Vector3(v.x, v.y, v.z); }
inline ogre::Quaternion from_ogre(const Ogre::Quaternion& q) {
    return ogre::Quaternion(q.w, q.x, q.y, q.z);
}
// PORT: a save record's orientation (core/saved_orientation.h) in OGRE's maths.
inline void save_orientation(const Ogre::Quaternion& turn, std::array<uint8_t, 64>& out) {
    tl::core::save_orientation(from_ogre(turn), out);
}
inline Ogre::Quaternion saved_orientation(const std::array<uint8_t, 64>& in) {
    return to_ogre(tl::core::saved_orientation(in));
}
namespace renderer_detail {
class RenderBehindMaterials;  // the render-behind outline's materials, below

using tl::core::kDefaultRimLight;

/// The game's own data, kept out of the default group so its scripts are not parsed
/// (mount_resources()).
constexpr const char* kGameGroup = "Torchlight";

/// Where place_mesh()'s increasing paint order for opaque layout meshes stops:
/// every later one shares this priority.
constexpr Ogre::ushort kLayoutDrawOrderCeiling = 90;

/// Path tests via stat() rather than std::filesystem: console paths carry a device prefix
/// ("sdmc:/..."), which std::filesystem::path is not specified for.
inline bool is_dir(const std::string& path) {
    struct stat st {};
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool is_file(const std::string& path) {
    struct stat st {};
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

/// Join two path components with '/', which every platform here accepts.
inline std::string join_path(const std::string& base, const std::string& leaf) {
    if (base.empty()) return leaf;
    const char back = base.back();
    return (back == '/' || back == '\\') ? base + leaf : base + "/" + leaf;
}

/// Forwards OGRE's log through ours, so the console's file sink captures it too.
class OgreLogBridge : public Ogre::LogListener {
public:
    void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool /*maskDebug*/,
                       const Ogre::String& /*logName*/, bool& /*skipThisMessage*/) override {
        // OGRE indexes resources by basename and Torchlight reuses filenames across
        // directories: these are expected, and would bury the real errors.
        if (message.find("because it already exists in resource group") != Ogre::String::npos) {
            ++duplicate_names;
            return;
        }

        tl::core::LogLevel level = tl::core::LogLevel::Info;
        switch (lml) {
            case Ogre::LML_TRIVIAL: level = tl::core::LogLevel::Debug; break;
            case Ogre::LML_NORMAL: level = tl::core::LogLevel::Info; break;
            case Ogre::LML_WARNING: level = tl::core::LogLevel::Warning; break;
            case Ogre::LML_CRITICAL: level = tl::core::LogLevel::Error; break;
        }
        tl::core::log_write(level, "OGRE", 0, message);
    }

    size_t duplicate_names = 0;
};

/// Generates a shader technique whenever a material has none for the RTSS scheme: the
/// game's materials are all fixed-function, which neither GL3Plus nor GLES2 has. Mirrors
/// OgreBites::SGTechniqueResolverListener (the Bites component is not built).
class ShaderResolver : public Ogre::MaterialManager::Listener {
public:
    ShaderResolver(Ogre::RTShader::ShaderGenerator* generator, GpuSkinning* skinning)
        : generator_(generator), skinning_(skinning) {}

    Ogre::Technique* handleSchemeNotFound(unsigned short /*schemeIndex*/,
                                          const Ogre::String& scheme_name, Ogre::Material* original,
                                          unsigned short /*lodIndex*/,
                                          const Ogre::Renderable* /*rend*/) override {
        if (!generator_->hasRenderState(scheme_name)) return nullptr;

        if (!generator_->createShaderBasedTechnique(*original, Ogre::MSN_DEFAULT, scheme_name)) {
            // Never silently: a null here draws the fixed-function technique, which stops
            // the frame, so it gets the blank white's. `createShaderBasedTechnique` refuses
            // when a redeclared material left an entry built from another source technique
            // (parse_material_script()).
            TL_LOG_WARN("no shader technique for material '%s' (group '%s'); it draws blank white",
                        original->getName().c_str(), original->getGroup().c_str());
            return blank_technique(scheme_name, original);
        }
        if (skinning_ != nullptr) skinning_->regenerating(*original);
        generator_->validateMaterial(scheme_name, *original);

        for (auto* technique : original->getTechniques()) {
            if (technique->getSchemeName() == scheme_name) return technique;
        }
        // The generator answered for the name and put nothing on this object:
        // its entry is another `Material` of the same name.
        TL_LOG_WARN(
            "the shader technique of material '%s' (group '%s') is another object's; it "
            "draws blank white",
            original->getName().c_str(), original->getGroup().c_str());
        return blank_technique(scheme_name, original);
    }

    bool afterIlluminationPassesCreated(Ogre::Technique* technique) override {
        if (!generator_->hasRenderState(technique->getSchemeName())) return false;
        Ogre::Material* material = technique->getParent();
        generator_->validateMaterialIlluminationPasses(technique->getSchemeName(),
                                                       material->getName(), material->getGroup());
        return true;
    }

    bool beforeIlluminationPassesCleared(Ogre::Technique* technique) override {
        if (!generator_->hasRenderState(technique->getSchemeName())) return false;
        Ogre::Material* material = technique->getParent();
        generator_->invalidateMaterialIlluminationPasses(technique->getSchemeName(),
                                                         material->getName(), material->getGroup());
        return true;
    }

private:
    /// OGRE's unlit blank white, with shaders: what a material with no shader technique
    /// draws with (its fixed-function one would throw).
    Ogre::Technique* blank_technique(const Ogre::String& scheme_name,
                                     const Ogre::Material* original) {
        const Ogre::MaterialPtr blank =
            Ogre::MaterialManager::getSingleton().getDefaultMaterial(false);
        if (!blank || blank.get() == original) return nullptr;
        blank->load();
        for (int attempt = 0; attempt < 2; ++attempt) {
            for (auto* technique : blank->getTechniques()) {
                if (technique->getSchemeName() == scheme_name) return technique;
            }
            if (!generator_->createShaderBasedTechnique(*blank, Ogre::MSN_DEFAULT, scheme_name))
                break;
            generator_->validateMaterial(scheme_name, *blank);
        }
        return nullptr;
    }

    Ogre::RTShader::ShaderGenerator* generator_;
    GpuSkinning* skinning_;
};

}  // namespace renderer_detail

using tl::core::MeshPlacement;

using tl::core::kUnitIdleAnimation;

using tl::core::ParticleStart;

using tl::core::kMissileParticleStopSeconds;

struct Renderer::Impl : tl::core::World {
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool add_map_portal(const ogre::Vector3& at, const std::string& warp, bool enabled) override {
        const auto r = add_map_portal(to_ogre(at), warp, enabled);
        return r;
    }
    using World::place_breakable;
    // PORT: `World::place_breakable` (core/world.h) in OGRE's maths, for the renderer's callers.
    bool place_breakable(const std::string& unit, const Ogre::Vector3& at,
                         const Ogre::Quaternion& facing, int64_t id, const Ogre::Vector3* settle,
                         const float* drawn_scale = nullptr) {
        const ogre::Vector3 settle_c = settle != nullptr ? from_ogre(*settle) : ogre::Vector3::ZERO;
        return World::place_breakable(unit, from_ogre(at), from_ogre(facing), id,
                                      settle != nullptr ? &settle_c : nullptr, drawn_scale);
    }
    using World::cast_prop_trap;
    // PORT: `World::cast_prop_trap` (core/world.h) in OGRE's maths, for the renderer's callers.
    void cast_prop_trap(const std::string& unit, const std::string& name, uint32_t level,
                        const Ogre::Vector3& skill_at, const Ogre::Quaternion& facing,
                        bool by_player) {
        World::cast_prop_trap(unit, name, level, from_ogre(skill_at), from_ogre(facing), by_player);
    }
    using World::drop_item;
    // PORT: `World::drop_item` (core/world.h) in OGRE's maths, for the renderer's callers.
    void drop_item(const tl::core::ItemStats& item, int count, const Ogre::Vector3& at) {
        World::drop_item(item, count, from_ogre(at));
    }
    using World::dropped_to_ground;
    // PORT: `World::dropped_to_ground` (core/world.h) in OGRE's maths, for the renderer's callers.
    Ogre::Vector3 dropped_to_ground(const Ogre::Vector3& at) const {
        const Ogre::Vector3 r = to_ogre(World::dropped_to_ground(from_ogre(at)));
        return r;
    }
    using World::place_gold;
    // PORT: `World::place_gold` (core/world.h) in OGRE's maths, for the renderer's callers.
    void place_gold(const tl::core::GoldPile& pile, const Ogre::Vector3& at,
                    const tl::core::ItemStats* item = nullptr, bool drop_sound = true) {
        World::place_gold(pile, from_ogre(at), item, drop_sound);
    }
    using World::place_ground_item;
    // PORT: `World::place_ground_item` (core/world.h) in OGRE's maths, for the renderer's callers.
    void place_ground_item(const tl::core::ItemStats& item, int count, const Ogre::Vector3& at,
                           const Ogre::Quaternion& orientation, const tl::core::QuestUnitTag& tag) {
        World::place_ground_item(item, count, from_ogre(at), from_ogre(orientation), tag);
    }
    using World::snap_item_to_ground;
    // PORT: `World::snap_item_to_ground` (core/world.h) in OGRE's maths, for the renderer's callers.
    Ogre::Vector3 snap_item_to_ground(const Ogre::Vector3& at) {
        const Ogre::Vector3 r = to_ogre(World::snap_item_to_ground(from_ogre(at)));
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    tl::core::QuestEventUnit quest_event_unit(const std::string& name, const std::string& unittype,
                                              const ogre::Vector3& at,
                                              const tl::core::QuestUnitTag& tag) override {
        const auto r = quest_event_unit(name, unittype, to_ogre(at), tag);
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    Ogre::SceneNode* view_place_entity(Ogre::Entity* entity, const ogre::Vector3& at,
                                       const ogre::Quaternion& turn) override {
        auto* const r = view_place_entity(entity, to_ogre(at), to_ogre(turn));
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    std::vector<Ogre::SceneNode*> place_particle_layout(const std::string& reference,
                                                        const ogre::Vector3& at,
                                                        const ogre::Quaternion& facing,
                                                        Ogre::SceneNode* parent,
                                                        ParticleStart start) override {
        const auto r =
            place_particle_layout(reference, to_ogre(at), to_ogre(facing), parent, start);
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    void view_aim_node(Ogre::SceneNode* node, const ogre::Vector3& at,
                       const ogre::Vector3& forward) override {
        view_aim_node(node, to_ogre(at), to_ogre(forward));
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    Ogre::SceneNode* view_create_node(const ogre::Vector3& at) override {
        auto* const r = view_create_node(to_ogre(at));
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool view_held_item(const Ogre::Entity* held, ogre::Vector3& hand,
                        float& height) const override {
        Ogre::Vector3 hand_o = to_ogre(hand);
        const auto r = view_held_item(held, hand_o, height);
        hand = from_ogre(hand_o);
        return r;
    }
    using World::add_combat_text;
    // PORT: `World::add_combat_text` (core/world.h) in OGRE's maths, for the renderer's callers.
    void add_combat_text(tl::core::CombatText what, const Ogre::Vector3& unit_at, int amount = 0) {
        World::add_combat_text(what, from_ogre(unit_at), amount);
    }
    using World::loot_placement_at;
    // PORT: `World::loot_placement_at` (core/world.h) in OGRE's maths, for the renderer's callers.
    tl::core::LootPlacement loot_placement_at(const Ogre::Vector3& at) {
        const auto r = World::loot_placement_at(from_ogre(at));
        return r;
    }
    using World::play_death_particle;
    // PORT: `World::play_death_particle` (core/world.h) in OGRE's maths, for the renderer's callers.
    void play_death_particle(const tl::core::UnitStats& unit, const Ogre::Vector3& at) {
        World::play_death_particle(unit, from_ogre(at));
    }
    using World::play_hit_particle;
    // PORT: `World::play_hit_particle` (core/world.h) in OGRE's maths, for the renderer's callers.
    void play_hit_particle(const tl::core::UnitStats& victim, const Ogre::Vector3& from,
                           const Ogre::Vector3& at) {
        World::play_hit_particle(victim, from_ogre(from), from_ogre(at));
    }
    using World::play_strike_particle;
    // PORT: `World::play_strike_particle` (core/world.h) in OGRE's maths, for the renderer's callers.
    void play_strike_particle(const tl::core::UnitStats& attacker, const Ogre::Vector3& from,
                              const Ogre::Vector3& at, float radius, bool critical, bool blocked) {
        World::play_strike_particle(attacker, from_ogre(from), from_ogre(at), radius, critical,
                                    blocked);
    }
    using World::show_absorbed;
    // PORT: `World::show_absorbed` (core/world.h) in OGRE's maths, for the renderer's callers.
    void show_absorbed(const Ogre::Vector3& at, const tl::core::UnitStats& stats) {
        World::show_absorbed(from_ogre(at), stats);
    }
    using World::start_death_shake;
    // PORT: `World::start_death_shake` (core/world.h) in OGRE's maths, for the renderer's callers.
    void start_death_shake(const char* name, float duration, float magnitude,
                           const Ogre::Vector3& centre) {
        World::start_death_shake(name, duration, magnitude, from_ogre(centre));
    }
    using World::unit_node_position;
    // PORT: `World::unit_node_position` (core/world.h) in OGRE's maths, for the renderer's callers.
    bool unit_node_position(int64_t node, Ogre::Vector3& at) {
        ogre::Vector3 at_c = from_ogre(at);
        const auto r = World::unit_node_position(node, at_c);
        at = to_ogre(at_c);
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool skill_bone_world(const ViewUnit& unit, int attachment, ogre::Vector3& out) override {
        Ogre::Vector3 out_o = to_ogre(out);
        const auto r = skill_bone_world(unit, attachment, out_o);
        out = from_ogre(out_o);
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool skill_unit_forward(const ViewUnit& unit, ogre::Vector3& out) override {
        Ogre::Vector3 out_o = to_ogre(out);
        const auto r = skill_unit_forward(unit, out_o);
        out = from_ogre(out_o);
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    void start_character_particle(const tl::core::UnitStats& unit, int slot,
                                  const ogre::Vector3& at) override {
        start_character_particle(unit, slot, to_ogre(at));
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    void view_stretch_lines(const std::vector<Ogre::SceneNode*>& line_holders,
                            const ogre::Vector3& start, const ogre::Vector3& end) override {
        view_stretch_lines(line_holders, to_ogre(start), to_ogre(end));
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    void view_translate(const std::vector<Ogre::SceneNode*>& holders,
                        const ogre::Vector3& by) override {
        view_translate(holders, to_ogre(by));
    }
    // PORT: the scene node hooks (core/world.h).
    ogre::Vector3 view_position(const Ogre::SceneNode* node) const override {
        return from_ogre(node->getPosition());
    }
    ogre::Quaternion view_orientation(const Ogre::SceneNode* node) const override {
        return from_ogre(node->getOrientation());
    }
    ogre::Vector3 view_derived_position(const Ogre::SceneNode* node) const override {
        return from_ogre(node->_getDerivedPosition());
    }
    ogre::Quaternion view_derived_orientation(const Ogre::SceneNode* node) const override {
        return from_ogre(node->_getDerivedOrientation());
    }
    void view_set_position(Ogre::SceneNode* node, const ogre::Vector3& at) override {
        node->setPosition(to_ogre(at));
    }
    void view_set_orientation(Ogre::SceneNode* node, const ogre::Quaternion& turn) override {
        node->setOrientation(to_ogre(turn));
    }
    Ogre::SceneNode* view_parent_node(const Ogre::Entity* entity) const override {
        return entity->getParentSceneNode();
    }
    tl::core::DungeonFiles archive_dungeon_files() const override;
    // PORT: the hook (core/world.h) in OGRE's maths.
    void play_effect_fx(tl::core::EffectManager& effects, const ogre::Vector3& centre,
                        const std::string& who) override {
        play_effect_fx(effects, to_ogre(centre), who);
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool play_sound(const std::string& name, const ogre::Vector3& at) override {
        const auto r = play_sound(name, to_ogre(at));
        return r;
    }
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool play_sound(const std::string& name, const ogre::Vector3& at, float volume_scale,
                    float radius_if_none, const std::string& voice_key,
                    bool volume_given) override {
        const auto r =
            play_sound(name, to_ogre(at), volume_scale, radius_if_none, voice_key, volume_given);
        return r;
    }
    // PORT: the archive hooks (core/world.h), answered by the archive the renderer mounts.
    std::vector<std::string> archive_paths_in_listing_order(const std::string& dir,
                                                            const char* suffix) const override;
    std::string resolve_archive_path_quiet(const std::string& reference) const override;
    std::string class_unit_path(const std::string& class_name, bool* console) const override;
    std::string archive_animation_in_dir(const std::string& dir, const std::string& state,
                                         bool armed, int variation) const override;
    std::string archive_path_in_dir(const std::string& basename,
                                    const std::string& dir) const override;
    std::string archive_path_unique(const std::string& basename) const override;
    std::vector<std::string> archive_paths_under(const std::string& dir,
                                                 const char* suffix) const override;
    std::vector<std::string> data_paths_under(const std::string& dir) const override;
    tl::formats::AdmFile parse_data_group(const uint8_t* data, size_t size) const override;
    bool console_game_data() const override;
    std::string dir_of_asset(const std::string& name) const override;
    tl::core::DungeonManager& loaded_dungeon_manager() const override;
    std::vector<uint8_t> read_resource(const std::string& path) const override;
    std::string resolve_archive_path(const std::string& reference) const override;

    Ogre::LogManager* log_manager = nullptr;
    std::unique_ptr<OgreLogBridge> log_bridge;

    std::unique_ptr<Ogre::Root> root;
#if defined(TL_PLATFORM_SWITCH) || defined(TL_PLATFORM_ANDROID) || defined(TL_PLATFORM_IOS)
    // One of the two per Root, never both: each makes its own EGL display.
    std::unique_ptr<Ogre::GLES2Plugin> gles2_plugin;
#if defined(TL_SWITCH_GL3PLUS)
    std::unique_ptr<Ogre::GL3PlusPlugin> gl3plus_plugin;
#endif
#else
    std::unique_ptr<Ogre::GL3PlusPlugin> gl3plus_plugin;
    std::unique_ptr<Ogre::GLPlugin> gl_plugin;
#endif

    /// ParticleUniverse, up while Root is. See `render/particle_universe.h`.
    ParticleUniverseHost particle_universe;

    Ogre::RenderWindow* window = nullptr;
    /// Android: the EGL surface let go in the background and remade, the context kept.
    void watch_window_surface();
    void fit_camera_to_window() const;  ///< to the window's first viewport, the world's
    Ogre::SceneManager* scene = nullptr;
    /// See `TransparentsWithScenery`.
    std::unique_ptr<Ogre::RenderQueueListener> transparents_listener;
    Ogre::Camera* camera = nullptr;
    Ogre::SceneNode* camera_node = nullptr;
    Ogre::Light* key_light = nullptr;
    Ogre::SceneNode* key_light_node = nullptr;
    Ogre::SceneNode* sky_node = nullptr;
    /// The template's `LEVELPARTICLE` layout (`CGameClient +0x1080`), put on the player
    /// every frame (`advance`).
    Ogre::SceneNode* level_particle_node = nullptr;

    /// Everything a layout placed, for the ground query below.
    std::vector<Ogre::Entity*> layout_meshes;

    /// Which of `layout_meshes` are units. See `MeshPlacement::is_unit`.
    std::set<Ogre::Entity*> unit_meshes;

    /// `stgeom_`, the room pieces' main batch (bake_static_scenery()); `stgeompre_` and
    /// `stgeompost_` are the scene manager's.
    Ogre::StaticGeometry* static_scenery = nullptr;
    /// The room pieces bake_static_scenery() may batch: bakeable, with the
    /// render phase their tileset gives them.
    struct BakeablePiece {
        Ogre::Entity* entity = nullptr;
        int render_phase = 0;
    };
    std::vector<BakeablePiece> bakeable_room_pieces;
    /// Numbers the sets' names, which a scene manager keeps unique.
    unsigned baked_sets = 0;

    /// The room pieces placed, whose materials take the light map's
    /// `LIGHTPASS` (see MeshPlacement::room_piece and add_light_pass()).
    std::vector<Ogre::Entity*> room_piece_meshes;

    /// The layout `Light`s' copies (MeshPlacement::light_density): light map only (`CLevel::bake`'s
    /// `stgeomLight_`; entities here).
    std::vector<Ogre::Entity*> light_pool_meshes;

    /// The layout's particle systems; see ParticleSystem.
    std::vector<ParticleSystem> particle_systems;

    ParticleIndex particle_index;
    void particle_index_add(size_t i);
    void particle_index_remove(size_t i);
    /// The systems one of `keys` (by link) or `wrapper_keys` (wrappers by node) names, in
    /// `particle_systems`' order, each once; then the lookups script events make.
    ParticleRefs indexed_particles(std::initializer_list<ParticleIndex::Key> keys,
                                   std::initializer_list<ParticleIndex::Key> wrapper_keys);
    ParticleRefs map_wrappers(const NodeMap& nodes);
    ParticleRefs link_systems(const NodeMap& nodes, int64_t node);
    ParticleRefs link_or_free_systems(const NodeMap& nodes, int64_t node);
    ParticleRefs node_wrappers(const NodeMap& nodes, int64_t node);
    ParticleRefs shown_systems(const NodeMap& nodes, int64_t node);
    size_t show_node_particles(const tl::core::ScriptEvent& event, NodeMap& nodes);

    /// Everything in the scene with an animation running: the level's NPCs and animated
    /// props. Apart from animated() (the subject), which runs at the player's pace.
    struct SceneAnimation {
        Ogre::Entity* entity = nullptr;
        float rate = 1.0f;
        /// A `Room Piece`'s model: `CRoomPiece::updateAnimation`
        /// runs it only while the camera sees
        /// its box (`room_piece_in_view`).
        bool room_piece = false;
    };
    /// `CRoomPiece::updateAnimation`'s test: position plus model box
    /// (`CGenericModel +0x1d4..+0x1e8`) inside the frustum (`Ogre::Frustum::isVisible`).
    ///
    bool room_piece_in_view(const Ogre::Entity* entity) const;
    std::vector<SceneAnimation> scene_animations;

    /// The track of `state` on `entity`'s animator, added on first use.
    unsigned entity_track(EntityAnimation& animation, Ogre::AnimationState* state);
    /// Copy the animator's tracks onto the states, where they changed.
    void write_entity_animation(EntityAnimation& animation) override;

    /// The window an NPC offers a quest in (`questdialogmenu_xenon.layout`): a
    /// `CQuestDialogMenu`, apart from a `FLOATYTEXT` line's `CTextEvent`.
    QuestDialogScreen quest_dialog;

    /// See `render/death_screen.h`.
    DeathScreen death_screen;
    /// The fishing ring (`media/UI/fishing_xenon.layout`, `CFishingMenu`, `CGameUI +0x3e4`),
    /// shown in AI state 0x21. `CFishingMenu::CFishingMenu` (X360 inline in
    /// `CGameUI::create`) starts shown (`+0x30`, X360 `+0x18`);
    /// `createMenus` builds it.
    LayoutScreen fishing_screen;
    /// Load Character (`media/UI/characterload.uilayout`, `CContinueGameMenu`),
    /// built when the screen is first asked for and drawn from
    /// `tl::core::ContinueMenu` by `show_load_character`.
    LayoutScreen character_load_screen;
    void show_load_character(const tl::core::ContinueMenu& state, const std::string& shown_file);
    /// `highlight`'s authored corner; `CContinueGameMenu +0x188` keeps it less
    /// `Player1`'s, which `show_load_character` takes off.
    float load_highlight_x = 0.0f, load_highlight_y = 0.0f;
    /// The modal message (`media/UI/modalmenu_xenon.layout`, `CModalMenu`, `CGameUI +0x3cc`)
    /// as `CGameUI::openModalDialog` opens it: a title, a line and `Ok`.
    /// `CModalMenu::CModalMenu`; no callbacks here.
    LayoutScreen modal_screen;

    /// `CQuestDialog::giveOrRemoveItems`: `[TAKEITEM]`s,
    /// then `[GIVEITEM]`s; false when already handed over (`+0x77`).
    bool hand_dialog_items(const tl::core::QuestDefinition& quest,
                           const tl::core::DialogLine& line) override;
    /// A floaty line the player's talk showed: `CQuestDialogMenu::update`
    /// calls `onClick(10)` on its next pass
    /// (`fresh` until then).
    struct TalkFloatyClose {
        bool pending = false;
        bool fresh = false;
        int machine = kNoMachine;
        int64_t node = 0;
        bool complete = false;
        std::string quest;
    };
    TalkFloatyClose talk_floaty_close;
    /// The placed or spawned unit called `unit` nearest the player, or 0.
    int64_t nearest_unit_node(const std::string& unit) const;

    /// Build the offer window for `unit` and show it; false when it has nothing to say or
    /// the console interface data is not mounted.
    bool open_quest_offer(const std::string& unit);
    /// `CCharacter::approachAI`'s choice for the unit reached, and what
    /// `CGameUI::updateIngameUI` opens for it.
    void interact_with(const std::string& unit);
    void open_vendor_screen(const std::string& unit, VendorScreen screen);
    void close_quest_offer() override;
    bool open_script_dialog(const tl::core::ScriptEvent& event, const std::string& text);
    /// The quest window is showing a line a `Quest Controller` opened, and
    /// answering it answers the script.
    bool script_dialog_window = false;

    /// `CQuestDialog::cleanUp` for a completed quest's
    /// lines: the pets they made leave the player.
    void release_quest_pets(const tl::core::QuestCompletion& done) override;

    /// `CQuestRewards::rewardPlayer` for one completion.
    void reward_player(const tl::core::QuestCompletion& done) override;

    /// The template's UNIT_FADE_DISTANCE and UNIT_FADE_START: where a unit's
    /// light falls to 0 and where it starts to (update_unit_lights()).
    float level_unit_fade_distance = 14.5f;
    float level_unit_fade_start = 7.0f;
    /// `KSETTINGS_F_AVERAGE_FPS`, kept as `CGame::update`
    /// keeps it.
    tl::core::AverageFps average_fps;
    /// The camera test `CCharacter::update` makes on the
    /// culling bounds (`culling_box`); no bounds counts as in view.
    bool unit_in_view(const tl::core::LevelScript& machine, int64_t node) override;
    /// A unit's culling bounds in the world (`CBaseUnit +0x1c0`, a
    /// `CCullingBounds`), sized from its model or the constructor's default.
    Ogre::AxisAlignedBox culling_box(Ogre::SceneNode* node, bool from_model) const;

    // The town portal (`core/town_portal.h`, renderer_portal.cpp).
    /// `CSoundObjectDescriptor::descriptorSceneActivated`
    /// for a portal's layout: `START ON ACTIVATE` sounds play.
    void start_portal_sounds(OpenPortal& portal);
    /// The held portal sounds' gain and pan, from where the listener stands.
    void step_portal_sounds();
    /// The gain and pan `play_sound` gives a definition heard at `at`; false
    /// when it is out of earshot.
    bool positional_mix(float volume, float radius, const Ogre::Vector3& at, float& gain,
                        float& pan) const;
    int portal_targeted = -1;
    /// `warpLevels("Town", -99)` from a portal used (`CTriggerUnit::trigger`).
    bool portal_warp_pending = false;
    tl::core::PortalLevel portal_level() const override;
    void delete_open_portals() override;
    void create_portals() override;
    /// `CCharacter::applyInstantEffect`'s `OPEN PORTAL` on the player:
    /// `CPlayer::openMapPortal`.
    void open_map_portal(const std::string& dungeon) override;
    /// `CResourceManager::createUnit(L"PROPS", L"MAP PORTAL")`, `CLevel::addItem` and its
    /// `WARP` effect: from `openMapPortal` and from a stored level state.
    bool add_map_portal(const Ogre::Vector3& at, const std::string& warp, bool enabled);
    /// What `setEnabled(false)` takes away from a portal prop: its particles
    /// and held sounds.
    void disable_portal(OpenPortal& portal);
    /// The level's portal props as `core/town_portal.h`'s two walks read them,
    /// one entry per `open_portals` entry and in its order.
    std::vector<tl::core::TemporaryPortal> temporary_portals() const;
    /// A on the targeted portal: `CTriggerUnit::trigger`.
    ///
    void use_targeted_portal();

    /// The Xbox `waypointmenu_xenon.layout`, and what `CWaypointMenu::setOpen`
    /// put in it.
    LayoutScreen waypoint_screen;
    /// `CGameUI::toggleWaypointMenu` ->
    /// `CWaypointMenu::setOpen`.
    void toggle_waypoint_menu();
    void open_waypoint_menu();
    void close_waypoint_menu() override;
    void refresh_waypoint_menu() override;
    // The Pause Menu and Options (`core/settings_menu.h`, renderer_options.cpp).
    /// `COptionsMenu` on the Xbox `options_Xenon.layout`: Start in the game.
    LayoutScreen pause_screen;
    bool pause_stick_held = false;
    /// `CSettingsMenuXenon` on `settingsmenu_xenon.layout`.
    LayoutScreen settings_screen;
    bool settings_stick_held = false;
    bool settings_dpad_held = false;
    bool settings_y_held = false;
    /// Y on Options: Help once the close has been carried out.
    bool help_after_settings = false;
    /// Help, `CHowToMenu` (`CGameUI +0x18b8`) on the Xbox `howto_xenon.layout`
    /// (`core/howto_menu.h`).
    LayoutScreen howto_screen;
    void create_options_screens();
    /// `CGameUI::toggleOptions`, opening.
    void open_pause_menu();
    void close_pause_menu() override;
    void refresh_pause_menu();
    /// The pad on the Pause Menu: the tab list, A is `onClick` on the row,
    /// Start (and B) close it.
    void drive_pause_menu(const tl::core::InputState& in, bool start_edge);
    /// `CGameUI::toggleSettings`, opening.
    void open_settings_menu();
    void refresh_settings_menu();
    void drive_settings_menu(const tl::core::InputState& in);
    /// `CSettingsMenuXenon::vfunc_3` once a frame: carries a close out, writes
    /// the file after Done and brings the Pause Menu back.
    void step_settings_menu();
    /// `CHowToMenu::setOpen`, and what its close leads to.
    void set_help_open(bool open) override;

    // Context tips (`core/context_tips.h`, renderer_tips.cpp).
    /// `CTipMenu` (`CGameUI +0x550`) on the Xbox `tipmenu_xenon.layout`.
    LayoutScreen tip_screen;
    /// `CGameUI +0x1960`.
    tl::core::TipQueue tip_queue;
    /// `CTipMenu +0x30`; `CTipMenu::CTipMenu`.
    bool tip_open = false;
    /// `CGameUI +0x1798`, the seven left-side menus `CGameUI::create` lists:
    /// `CInventoryMenuXenon`, `CSkillMenuXenon`, `CQuestMenuXenon`,
    /// `CMerchantMenuXenon`, `CEnchantMenuXenon`, `CStashMenuXenon`, `CStatsMenuXenon`; the
    /// port's master menu is all of them.
    bool full_screen_menu_up() const override { return inventory_menu.visible(); }
    /// Which of them, for the log.
    const char* modal_dialog_name() const;
    bool level_was_paused = false;
    /// The player's target is an interactive character (`ISA 0x83`) through its
    /// conversation: paused, `CGameClient::updateIngame`
    /// keeps the particles beside it running. A quest controller's window too
    /// (`CQuestController::interactWithUnit` -> `CGameUI::forceInteractiveDialog`
    /// `CCharacter::setTarget`).
    bool talking_to_unit() const {
        return talk_target_machine != kNoMachine ||
               (quest_dialog.visible() &&
                (!script_dialog_window || conversation_speaker() != nullptr));
    }
    /// That, or the quest dialog having its NPC: what `updateIngame` still
    /// updates while paused - …18777 tokens truncated…h as `loadAnimations` builds it from the
    /// `.ANIMATION` file (`tl::core::load_animation_list`); `missing` counts absent files.
    tl::core::AnimationCatalog model_catalog(const std::string& mesh, const std::string& dir,
                                             size_t* missing = nullptr) override;
    /// Every placed unit's cast: its animation's keys, `updateActiveSkill` and
    /// its manager's `update`.
    void step_unit_skills(float delta);
    /// The entity a machine's unit is drawn with, or null.
    SceneUnit* scene_unit_of(tl::core::LevelScript* machine, int64_t node) override;
    // PORT: drawing, a skill event's particle systems (renderer_effects.cpp).
    size_t view_particle_mark() const override;
    void view_ride_particles(size_t mark, Ogre::SceneNode* ride) override;
    void view_claim_particles(size_t mark, std::vector<Ogre::SceneNode*>& holders,
                              std::vector<Ogre::SceneNode*>& line_holders) override;
    bool view_particles_alive(const std::vector<Ogre::SceneNode*>& holders) const override;
    void view_stop_particles(const std::vector<Ogre::SceneNode*>& holders, bool loop) override;
    void view_stop_emission(const std::vector<Ogre::SceneNode*>& holders) override;
    void view_translate(const std::vector<Ogre::SceneNode*>& holders, const Ogre::Vector3& by);
    void view_stretch_lines(const std::vector<Ogre::SceneNode*>& line_holders,
                            const Ogre::Vector3& start, const Ogre::Vector3& end);
    /// `CSkillEvent::getBonePosition(unit, attachment, true)`
    /// in the world; false when the unit is gone.
    bool skill_bone_world(const ViewUnit& unit, int attachment, Ogre::Vector3& out);
    /// The unit's facing on the ground (vtable +0x130). False when it is gone.
    bool skill_unit_forward(const ViewUnit& unit, Ogre::Vector3& out);

    /// `CCharacter::doWeaponSkill(weapon)`: true when a
    /// skill started, which ends the blow.
    bool player_weapon_skill(const ProcTarget& target) override;

    // PORT: drawing, a missile's node and particles, a held item's model (renderer_effects.cpp).
    Ogre::SceneNode* view_create_node(const Ogre::Vector3& at) const;
    void view_aim_node(Ogre::SceneNode* node, const Ogre::Vector3& at,
                       const Ogre::Vector3& forward);
    void view_destroy_node(Ogre::SceneNode* node) override;
    void view_transients_alive(const std::vector<const std::vector<Ogre::SceneNode*>*>& lists,
                               const std::vector<char>& stop, std::vector<char>& alive) override;
    bool view_held_item(const Ogre::Entity* held, Ogre::Vector3& hand, float& height) const;
    /// A particle layout placed at `at` (or riding `parent`), and its systems' nodes.
    std::vector<Ogre::SceneNode*> place_particle_layout(const std::string& reference,
                                                        const Ogre::Vector3& at,
                                                        const Ogre::Quaternion& facing,
                                                        Ogre::SceneNode* parent,
                                                        ParticleStart start = ParticleStart::Now);
    void destroy_particle_holders(std::vector<Ogre::SceneNode*>& holders) override;
    /// `CBaseUnit::sphereCollision` for a drawn unit: the
    /// first `t` in [0, 1] its collision model is touched, above 1 if not; false if not drawn.
    bool unit_collider_sweep(int64_t id, const tl::core::Vec3& from, const tl::core::Vec3& to,
                             float radius, float& t) override;
    /// A character's collision model (`CCharacter +0x4c8`): `media/models/collider.mesh`,
    /// what `loadModel` gives units with no `COLLISIONFILE`.
    tl::core::CollisionWorld unit_collider;
    bool unit_collider_read = false;

    void follow_subject();
    /// `CCameraControl::updateGameCamera` for the engine's default framing.
    void update_game_camera(const Ogre::Vector3& player_at);
    void step_player(float delta, const tl::core::InputState& in) override;
    void drive_player(float delta);
    Ogre::Entity* place_mesh(const MeshPlacement& placement);
    /// `CLevel::bake`'s room-piece batching, once the level and its
    /// remembered state are in place.
    void bake_static_scenery() override;
    void view_prepare_mesh(const std::string& mesh_path) override;

    /// Carry out what a level script decided, and log it; `nodes` is the map the events
    /// address.
    void apply_script_events(const std::vector<tl::core::ScriptEvent>& events,
                             NodeMap& nodes) override;

    /// One frame of the level script, from wherever the character is.
    void step_script(float delta);

    // The level remembered and put back.

    /// A `Cinematic` node's `[CINEMATIC]`, opened by the run loop.
    std::string pending_layout_cinematic;

    // PORT: the automap's revealed cells into a level state and back (renderer_session.cpp).
    void view_store_automap(tl::formats::LevelSaveState& state) override;
    void view_restore_automap(const tl::formats::LevelSaveState& state) override;

    /// Put the level back as it loaded: nothing hidden, no camera held, triggers unspent.
    void reset_script();
    /// `CParticle::Start`'s timer over
    /// `particle_systems[first, last)`: largest `LIFE TIME`, else largest finite emitter
    /// window, none if any never expires; plus 0.1.
    void arm_particle_timer(size_t first, size_t last);
    bool place_particle(const tl::formats::CmpScene& layout, const tl::formats::CmpNode& node,
                        const Ogre::Vector3& position, const Ogre::Quaternion& orientation);
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool place_particle(const tl::formats::CmpScene& layout, const tl::formats::CmpNode& node,
                        const ogre::Vector3& position,
                        const ogre::Quaternion& orientation) override {
        return place_particle(layout, node, to_ogre(position), to_ogre(orientation));
    }
    void view_automap_add_tile(int type, const ogre::Vector3& at, const ogre::Vector3& dir, bool a,
                               bool b) override;
    void view_automap_player(const ogre::Vector3& at) override;
    void view_place_viewer(const tl::formats::CmpScene& layout,
                           const ogre::Vector3& origin) override;
    void view_place_meshes(const std::vector<MeshPlacement>& placements, size_t& animated_props,
                           size_t& idled) override;
    /// One of `view_place_meshes`' placements: its entity, its animation and, a unit's, its
    /// scene entry.
    void view_place_one_mesh(const MeshPlacement& from_layout, size_t& animated_props,
                             size_t& idled);
    /// A placed unit's scene entry, hands, render flags and rim light.
    void dress_placed_unit(Ogre::Entity* entity, const MeshPlacement& placement,
                           Ogre::AnimationState* started, bool armed);
    void view_own_trigger_particles(size_t before, int64_t node, const NodeMap& sink) override;
    void view_settle_link_particles(size_t systems_before, int64_t owner,
                                    const std::string& reference, const ogre::Vector3& origin,
                                    const ogre::Quaternion& basis,
                                    tl::core::LinkParticleSettle what) override;
    /// `view_settle_link_particles`' parts: its systems learn their link; they become one
    /// `CParticle`; they stop.
    void link_particle_systems(size_t systems_before, int64_t owner, const Ogre::Vector3& origin,
                               const Ogre::Quaternion& basis);
    void register_link_cparticle(size_t systems_before, int64_t owner, const std::string& reference,
                                 const Ogre::Vector3& origin, bool stopped);
    void stop_particle_systems(size_t systems_before);
    bool view_caching_particles() const override;
    unsigned long view_milliseconds() const override;
    void view_automap_create() override;
    size_t view_level_particle_node(size_t systems_before) override;
    void view_stand_subject(const ogre::Vector3& at, const ogre::Vector3& facing) override;
    void logic_trace_text(const char* text) override;
    /// One `Particle` node as Torchlight's ParticleUniverse system, started as
    /// `CParticle::Start` starts it.
    bool place_torchlight_particle(const tl::formats::CmpScene& layout,
                                   const tl::formats::CmpNode& node, const Ogre::Vector3& position,
                                   const Ogre::Quaternion& orientation);
    /// `CParticle::updateLevelObject` for a system.
    void step_torchlight_particle(ParticleSystem& system, float delta);
    void step_particles(float delta);

    /// One frame of the idle fidgets: see `Impl::Fidget`.
    void step_fidgets(float delta, float world_delta);

    /// Start a fidget on one entity, or say there is nothing to start.
    bool begin_fidget(Fidget& fidget, Ogre::Entity* entity, const std::string& dir, bool is_subject,
                      const std::string& mesh, bool armed);
    /// `started`: the state enabled, the only way to switch that animation off later.
    /// `played`: the animation class actually started (RUN/WALK may substitute).
    /// `blend`: 0 plays outright (`CGenericModel::playAnimation`), else cross-fades
    /// (`blendAnimation`). `queue`: waits behind what plays (`queueBlendAnimation`).
    bool play_scene_animation(Ogre::Entity* entity, const std::string& mesh_path,
                              const std::string& animation, bool loops, float rate,
                              bool optional = false, Ogre::AnimationState** started = nullptr,
                              bool armed = true, std::string* played = nullptr, float blend = 0.0f,
                              bool queue = false) override;

    /// How long a placed unit's animation runs (`CGenericModel::getAnimationLengthSeconds`);
    /// negative when missing, 0 for a pose.
    float scene_animation_length(const std::string& unit, const std::string& animation) override;

    /// Append what a machine traced to `--logic-trace`.
    void flush_logic_trace(tl::core::LevelScript& machine) override {
        if (logic_trace == nullptr) return;
        for (const std::string& line : machine.take_trace()) {
            std::fputs(line.c_str(), logic_trace);
            std::fputc('\n', logic_trace);
        }
        std::fflush(logic_trace);
    }
    void aim_camera(const tl::formats::CmpScene& scene, const Ogre::Vector3& origin);
    bool ground_under(const Ogre::Vector3& from, float& surface_y) const;
    // PORT: the hook (core/world.h) in OGRE's maths.
    bool ground_under(const ogre::Vector3& from, float& surface_y) const override {
        return ground_under(to_ogre(from), surface_y);
    }
    void stand_subject_at(const tl::formats::CmpNode& start, const Ogre::Vector3& origin);
    /// The layout is a main-menu backdrop (`/MAINMENUS/`), loaded via
    /// `CGameClient::loadMenuLevel`: `CLevel::bake` adds its `Light`s to
    /// `stgeomLight_`, but only `CGameClient::loadLevel`'s pass
    /// builds it, so a menu's layout lights never reach the light map.
    bool loading_menu_level = false;
    void apply_environment(const std::string& layout_path) override;
    void set_model_ambient(Ogre::Entity* entity, const Ogre::ColourValue& colour,
                           std::set<Ogre::Material*>& seen);
    void apply_scenery_fill(const tl::formats::AdmSection& rules);
    /// The level template's `MATERIAL AMBIENT` (`CLevelTemplateData +0x880`),
    /// as apply_scenery_fill() read it.
    Ogre::ColourValue material_ambient = Ogre::ColourValue(0.36078432f, 0.36078432f, 0.36078432f);
    void setup_shadow_map(const tl::formats::AdmSection& rules);
    /// Logs what the scene holds (nodes, entities, particle systems...), with
    /// `when` after "scene" in the line.
    void log_scene_census(const char* when) const override;
    /// The end of a timing window: glthread's costs, the shaders made, the GPU
    /// waits (renderer_diagnostics.cpp).
    static void log_driver_costs(bool print, unsigned frames);
    void update_player_shadow(const Ogre::Vector3& focus);
    /// Every character's and loose item's light by distance from `focus` (`CCharacter::update`
    /// `CItem::update`, `CGenericModel::setLightOverride`),
    /// once a frame.
    void update_unit_lights(const Ogre::Vector3& focus);
    /// `COLLISION_RADIUS` by unit name, for the fade's sphere centre.
    std::unordered_map<std::string, float> unit_radius_cache;
    void destroy_shadow_map();
    void setup_light_map(const tl::formats::AdmSection& rules);
    void add_light_pass(const Ogre::MaterialPtr& material);
    void step_light_map(float dt);
    void aim_light_map(const Ogre::Vector3& focus);
    void destroy_light_map();
    bool setup_shader_generator();
    /// The program binaries `config.shader_cache` holds, into OGRE and back out before it
    /// goes (RendererConfig::shader_cache).
    void load_shader_cache();
    void save_shader_cache();
    /// Where those go: `config.shader_cache`, or a file of its own while the console runs
    /// GL3Plus (RTSS writes different GLSL per render system).
    std::string shader_cache_file;
    void frame_camera_on(Ogre::Entity* entity);
    void apply_scheme(Ogre::Viewport* viewport) const;
    /// What the view is cleared to: the port's grey for a bare model, else the level's fog
    /// (`CGameClient::refreshLighting`).
    Ogre::ColourValue view_background{0.15f, 0.16f, 0.20f};
    void update_automap_icons() override;
    /// `CCharacter::updateMotion`'s icon move and `setMeshVisible`'s visibility, per frame.
    void step_automap_icons();
    /// A unit's automap icon (`CCharacter +0x728`), and whether its mesh was
    /// visible the last time it was looked at.
    struct UnitIcon {
        Ogre::SceneNode* node = nullptr;
        Ogre::Entity* entity = nullptr;
        Ogre::Billboard* billboard = nullptr;
        bool shown = true;
        std::string unit;
        Ogre::Vector3 placed = Ogre::Vector3::ZERO;
        bool reported_moving = false;
    };
    std::vector<UnitIcon> unit_icons;
    size_t last_unit_icon_count = static_cast<size_t>(-1);
};
namespace renderer_detail {
/// Declared here because mounting the archive invalidates the caches.
void forget_archive_index();

void forget_parsed_materials();

/// Parses the material script beside a mesh (same basename), not the whole archive's.
std::string dir_of_asset(const std::string& name);

std::vector<std::string> declared_materials(const std::string& text);
std::string qualify_material_names(const std::string& text,
                                   const std::vector<std::pair<std::string, std::string>>& renames);
std::string strip_material_blocks(const std::string& text, const std::vector<std::string>& drop);
std::string deduplicate_material_script(const std::string& text, size_t& dropped);
std::string complete_colour_op_ex(const std::string& text, size_t& fixed);
extern std::set<std::string> g_parsed_materials;
/// How often one script may be parsed before the log warns (a cache being thrashed).
constexpr size_t kNoisyParseCount = 8;

extern std::map<std::string, size_t> g_parse_counts;
extern std::map<std::string, std::string> g_material_home;
extern std::map<std::string, std::string> g_material_alias;
extern std::map<std::string, std::string> g_material_script;
void forget_parsed_materials();
// Defined below, next to the archive index it reads.
size_t material_script_rank(const std::string& script);

extern Ogre::SceneManager* g_material_scene;
// Defined below, next to the other things that know about materials.
size_t rebind_materials(Ogre::Entity* entity);
/// `CGenericModel::reInitialize`'s fully emissive passes drawn unlit (renderer.cpp).
void unlight_full_emissive(Ogre::Entity* entity);

size_t rebind_scene_materials(Ogre::SceneManager* scene);

void apply_qualified_materials(Ogre::Entity* entity, const std::string& dir);

// Defined below, next to the other things that know about the wardrobe's naming.
std::string slot_of_material(const std::string& material_name);

void qualify_texture_paths(const Ogre::MaterialPtr& material, const std::string& dir,
                           const std::string& fallback_dir);

std::string dir_of_asset(const std::string& name);

std::string fill_empty_texture_units(const std::string& text, const std::string& script,
                                     size_t& filled);

void parse_material_script(const std::string& script, std::vector<std::string>* redefined = nullptr,
                           bool force = false);
extern std::unordered_map<std::string, std::vector<Ogre::FileInfo>> g_archive_index;
extern std::unordered_map<std::string, std::vector<std::string>> g_materials_by_dir;
extern bool g_archive_indexed;
bool ends_with_lower(const std::string& text, const char* suffix);
std::string lowered(const std::string& text);
void forget_archive_index();
void ensure_archive_index();
size_t material_script_rank(const std::string& script);
const std::vector<Ogre::FileInfo>* archive_entries(const std::string& basename);
std::vector<std::string> archive_paths_under(const std::string& dir, const char* suffix);
std::vector<std::string> archive_paths_in_listing_order(const std::string& dir, const char* suffix);
/// The file of the spawn class whose `NAME` is `name` (case aside), or empty.
std::vector<std::string> underscore_words(const std::string& name);
std::string wardrobe_texture_for(const std::string& mesh_name, const std::string& dir,
                                 const std::string& slot, const std::string& set_word);
size_t rebind_materials(Ogre::Entity* entity);
Ogre::TextureUnitState* rim_unit(Ogre::Pass* pass, const std::string& texture);
void apply_rim_light(Ogre::Entity* entity, const std::string& texture);
void apply_render_always(Ogre::Entity* entity, bool render_always);
/// A mesh's bounds as shipped: apply_render_always() replaces a
/// `RENDERALWAYS` mesh's own with the original's never-culled box.
const Ogre::AxisAlignedBox& authored_mesh_bounds(const Ogre::Entity* entity);
void highlight_entity(Ogre::Entity* entity, bool on, bool hostile, const std::string& rim_texture);
void apply_qualified_materials(Ogre::Entity* entity, const std::string& dir);
size_t rebind_scene_materials(Ogre::SceneManager* scene);
size_t report_stale_materials(Ogre::SceneManager* scene, const char* when);
void repoint_materials(const std::vector<Ogre::Entity*>& entities,
                       const std::vector<std::string>& redefined, const std::string& body_dir);
void parse_sibling_material(const std::string& mesh_name,
                            std::vector<std::string>* redefined = nullptr, bool force = false);
std::string archive_dir_of(const std::string& resource_name);
std::string archive_path_in_dir(const std::string& basename, const std::string& dir);
std::string animation_state_of(const std::string& stem, bool& armed, bool& has_armed_suffix,
                               int& variation);
std::string archive_animation_in_dir(const std::string& dir, const std::string& state, bool armed,
                                     int variation);
std::vector<std::string> archive_animation_stems_matching(const std::string& dir,
                                                          const std::string& substring);
std::string class_unit_path(const std::string& class_name, bool* console = nullptr);
std::string archive_path_unique(const std::string& basename);
/// Find the file a material's texture name refers to: by the mesh, then the wearer, then
/// anywhere unambiguous, then with the archive's extensions; never an ambiguous match.
std::string resolve_archive_path(const std::string& reference);
/// The same lookup without the warning, to ask whether a name is a full archive path.
std::string resolve_archive_path_quiet(const std::string& reference);

std::string resolve_texture(const std::string& name, const std::string& dir,
                            const std::string& fallback_dir);
void qualify_animated_frames(Ogre::TextureUnitState* unit, const std::string& dir,
                             const std::string& fallback_dir);
void qualify_texture_paths(const Ogre::MaterialPtr& material, const std::string& dir,
                           const std::string& fallback_dir);
std::string dir_of_asset(const std::string& name);
std::string animation_named(const Ogre::Entity* entity, const std::string& wanted);
std::string slot_of_material(const std::string& material_name);
std::string fill_empty_texture_units(const std::string& text, const std::string& script,
                                     size_t& filled);
void ensure_skeleton(const Ogre::MeshPtr& mesh, const std::string& mesh_name,
                     const std::string& dir);
std::string resolve_archive_path(const std::string& reference);
/// Keep read_resource()'s data files only while the same archives are mounted.
void remember_mounts(const std::string& mounts);
/// Absolute roots of extracted mod folders, in priority order; these filesystems
/// are searched case-insensitively before the base archive on iOS.
void set_mod_roots(const std::vector<std::string>& roots);
bool read_mod_resource(const std::string& path, std::vector<uint8_t>& bytes);
std::vector<std::string> mod_paths_under(const std::string& dir, const char* suffix);
std::vector<std::string> data_paths_under(const std::string& dir);
tl::formats::AdmFile parse_data_group(const uint8_t* data, size_t size);
std::vector<std::string> data_paths_in_listing_order(const std::string& dir);
std::vector<uint8_t> read_resource(const std::string& path);
/// The pet a new character starts with unless the creation screen says otherwise
/// (`charactercreate.layout` lists `Dog` first).
constexpr const char* kMenuPetUnit = "Dog";

/// Where a class's idle is parked on the creation screen (as `--animation-time`).
constexpr float kMenuIdleTime = 1.1f;

using tl::core::kMenuMusic;

std::vector<uint8_t> read_file(const std::string& path);
void log_animation_states(const Ogre::Entity* entity);
using tl::core::names_the_bind_pose;
bool skeleton_has_tracks(const Ogre::SkeletonPtr& skeleton);
size_t pose_from_skeleton(Ogre::Entity* entity, const Ogre::SkeletonPtr& pose);
size_t merge_shared_animations(const Ogre::SkeletonPtr& master, const Ogre::Skeleton* source,
                               const Ogre::Skeleton::BoneHandleMap& bones,
                               const Ogre::StringVector& wanted);
bool link_path_onto(Ogre::Entity* entity, const std::string& path);

/// Draws the transparent scenery OGRE 14 moves to `RENDER_QUEUE_TRANSPARENTS` (95) as the
/// last thing in the main group, where Torchlight's OGRE 1.6 drew it (particles sit at 51
/// to 90; `CParticleTechWrapper::setAsLight` 0x5a + sort index). Group 95 is drawn just
/// before the first group after the main one, then skipped.
class TransparentsWithScenery : public Ogre::RenderQueueListener {
public:
    explicit TransparentsWithScenery(Ogre::SceneManager* scene) : scene_(scene) {}

    void preRenderQueues() override { drawn_ = false; }

    void renderQueueStarted(Ogre::uint8 id, const Ogre::String&, bool& skip) override {
        if (id == Ogre::RENDER_QUEUE_TRANSPARENTS) {
            skip = drawn_;
            return;
        }
        if (drawn_ || id <= Ogre::RENDER_QUEUE_MAIN || id > Ogre::RENDER_QUEUE_TRANSPARENTS) return;
        drawn_ = true;
        const auto& groups = scene_->getRenderQueue()->_getQueueGroups();
        if (Ogre::RenderQueueGroup* group = groups[Ogre::RENDER_QUEUE_TRANSPARENTS].get()) {
            if (scene_->isRenderQueueToBeProcessed(Ogre::RENDER_QUEUE_TRANSPARENTS)) {
                scene_->_renderQueueGroupObjects(group,
                                                 Ogre::QueuedRenderableCollection::OM_PASS_GROUP);
            }
        }
    }

private:
    Ogre::SceneManager* scene_;
    bool drawn_ = false;
};

/// What the view queues (installed by setup_scene()). The light map's groups 95-99 are
/// never drawn in the view: `CGame::renderableQueued` (installed by
/// `CGame::preViewportUpdate`; removed) refuses
/// them (`CGenericModel::setRenderToLightMap`;
/// `CParticleTechWrapper::setAsLight`; `CGameClient::renderableQueued`,
/// `preRenderTargetUpdate`); here at
/// kLightMapGroupFirst..kLightMapGroupLast (kVisibilityLightMap). And the merged scenery
/// is re-queued at `kLayoutDrawOrderCeiling` (90) so depth ties fall as before the merge.
/// The original's queues: group 0x58 (characters; `CCharacter::setRenderBehind`, 50 with
/// the body re-queued) after the main group, then Runic's 0x5a up. Below 0x58 they are
/// OGRE's numbers; from 0x58 on they follow the main group, capped below the overlay.
constexpr unsigned runic_queue(unsigned queue) {
    return queue < 0x58 ? queue
                        : (queue - 0x58 + Ogre::RENDER_QUEUE_MAIN + 1 < 90
                               ? queue - 0x58 + Ogre::RENDER_QUEUE_MAIN + 1
                               : 90u);
}

constexpr unsigned kCharacterGroup = runic_queue(0x58);

/// Where Runic's group 0x5a lands; the rest follow at the same offsets.
constexpr unsigned kParticleGroupBase = runic_queue(0x5a);

constexpr unsigned kLightMapGroupFirst = kParticleGroupBase + 5;

constexpr unsigned kLightMapGroupLast = kParticleGroupBase + 9;

/// What the light map draws: things with its visibility flag. `preRenderTargetUpdate`
/// masks to 2, carried only by the plate and mask quads,
/// the `stgeomLight_` lights (`CLevel::bake`) and a
/// `setRenderToLightMap(true)` model; everything else
/// carries 1 (`CGame::chooseSceneManager`), particles included. The port's 2 is
/// the automap's, so here it is 4.
constexpr std::uint32_t kVisibilityLightMap = 0x4;

/// A renderable whose material has no compiled technique is queued with OGRE's blank white
/// (`RenderQueue::addRenderable`) and stays on it, as the queue only `prepare()`s it. The
/// shader generator rewriting its shaders (`SGScheme::synchronizeWithLightSettings`,
/// `Material::removeTechnique`) leaves a prepared material so; it is compiled here.
inline void compile_waiting_material(Ogre::Renderable* rend, Ogre::Technique** technique) {
    if (technique == nullptr || *technique == nullptr) return;
    const Ogre::MaterialPtr& held = rend->getMaterial();
    if (!held || (*technique)->getParent() == held.get()) return;
    if (!held->getCompilationRequired() || !held->getSupportedTechniques().empty()) return;
    held->touch();
    if (Ogre::Technique* own = rend->getTechnique()) {
        *technique = own;
        TL_LOG_INFO("material '%s' was left with no technique compiled; compiled again",
                    held->getName().c_str());
    }
}

/// `CCharacter::setRenderBehind(bool)`: a model drawn
/// through what stands in front of it (`CGenericModel::setRenderBehind`);
/// queue 50 on, 0x58 off; equipped items follow
/// (`CEquipment::setRenderBehind`). `hostile`: `CCharacter::setAlignment`'s
/// `outlinered.dds` over `outlineblue.dds`.
struct RenderBehind {
    bool on = false;
    bool hostile = false;
};
void set_render_behind(Ogre::Entity* entity, bool on, bool hostile);
/// The state `set_render_behind` left on an entity; null when it never ran.
const RenderBehind* render_behind_of(const Ogre::Entity* entity);

/// The render-behind materials by colour and skinning (`CGenericModel::reInitialize`'s
/// `rbmat_N`).
class RenderBehindMaterials {
public:
    explicit RenderBehindMaterials(GpuSkinning* gpu) : gpu_(gpu) {}
    Ogre::MaterialPtr material(bool hostile, unsigned bones, unsigned weights);

private:
    GpuSkinning* gpu_;
    std::map<std::tuple<bool, unsigned, unsigned>, Ogre::MaterialPtr> materials_;
};

/// `CGame::renderableQueued`, its render-behind half: a
/// seen sub-entity is queued again on 0x58 (the body) and drawn here additive with depth
/// test `GREATER`. True when it swapped `technique`.
bool render_behind_swap(Ogre::Renderable* rend, Ogre::ushort priority, Ogre::Technique** technique,
                        Ogre::RenderQueue* queue, RenderBehindMaterials& materials,
                        GpuSkinning* gpu);

class ViewRenderables : public Ogre::RenderQueue::RenderableListener {
public:
    bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 group, Ogre::ushort priority,
                          Ogre::Technique** technique, Ogre::RenderQueue* queue) override {
        compile_waiting_material(rend, technique);
        if (group >= kLightMapGroupFirst && group <= kLightMapGroupLast) return false;
        if (gpu != nullptr) gpu->guard(rend, technique);
        // In play only: `CGameClient +0x38d0` (the game state) 0 refuses it.
        if (behind != nullptr && behind_in_play && group == Ogre::RENDER_QUEUE_MAIN &&
            render_behind_swap(rend, priority, technique, queue, *behind, gpu)) {
            return true;
        }
        if (priority == kLayoutDrawOrderCeiling || opaque == nullptr) return true;
        auto* bucket = dynamic_cast<Ogre::StaticGeometry::GeometryBucket*>(rend);
        if (bucket == nullptr ||
            bucket->getParent()->getParent()->getParent()->getParent() != opaque) {
            return true;
        }
        queue->addRenderable(rend, group, kLayoutDrawOrderCeiling);
        return false;
    }

    /// The level's merge; nothing else is re-queued.
    const Ogre::StaticGeometry* opaque = nullptr;
    /// No skinning shader on what the CPU skins (GpuSkinning::guard).
    GpuSkinning* gpu = nullptr;
    /// The render-behind outline's materials, and whether the game is in play.
    RenderBehindMaterials* behind = nullptr;
    bool behind_in_play = false;
};

extern ViewRenderables g_view_renderables;
bool load_archive_image(const std::string& path, Ogre::Image& out);
}  // namespace renderer_detail
Ogre::Quaternion orientation_from_basis(const Ogre::Vector3& forward, const Ogre::Vector3& right,
                                        const Ogre::Vector3& up);
Ogre::Quaternion node_orientation(const tl::formats::CmpNode& node);
Ogre::Vector3 layout_scale(const tl::formats::CmpNode& node, float unit_scale);
Ogre::MeshPtr load_layout_mesh(const std::string& mesh_path);
Ogre::AnimationState* state_named_any_of(Ogre::AnimationStateSet* states,
                                         const std::vector<std::string>& wanted);
namespace renderer_detail {
/// `g_bCachParticles`: set by the first level a template loads
/// (`CLevel::loadRoomLayout`,
/// `CLayout::setCacheingParticlesForLevel`), never cleared, for the process.
bool caching_particles();
void set_caching_particles();
void restart_particle_system(ParticleSystem& system);
void stop_particle_emission(ParticleSystem& system);
using tl::core::kDefaultCameraDistance;

Ogre::Vector3 default_camera_offset(float camera_mult);
using tl::core::find_spawn;
using tl::core::log_memory;
/// The material name of every submesh as the serializer reads it, in order: OGRE 14 keeps
/// no unresolvable name (`SubMesh::getMaterialName`), and collision materials
/// (`tl::core::collision_material`) resolve nowhere (`MeshSerializerImpl::readSubMesh`).
struct SubmeshMaterialNames final : Ogre::MeshSerializerListener {
    std::vector<std::string> names;
    void processMaterialName(Ogre::Mesh*, Ogre::String* name) override { names.push_back(*name); }
    void processSkeletonName(Ogre::Mesh*, Ogre::String*) override {}
    void processMeshCompleted(Ogre::Mesh*) override {}
};

using tl::core::log_passability;
using tl::core::write_passability_dump;
void for_each_triangle(
    const Ogre::MeshPtr& mesh, const Ogre::Matrix4& to_world,
    const std::function<void(const Ogre::Vector3&, const Ogre::Vector3&, const Ogre::Vector3&)>& fn,
    const std::function<bool(const std::string&)>& accept_material = nullptr);
}  // namespace renderer_detail
using tl::core::kSpawnedNodeBase;

using tl::core::kMissilePlayer;

namespace renderer_detail {
using tl::core::spawn_data_of;
Ogre::Quaternion quaternion_from_axes(const tl::core::SpawnAxes& axes);
/// `ProjectorCam` (`CGameClient::CGameClient`): orthographic,
/// `setFrustumExtents(-24, 24, 24, -24)`, looking straight down, kept 100 above the focus
/// (`CCameraControl::updateGameCamera`).
constexpr float kLightMapHalfExtent = 24.0f;

constexpr float kLightMapNear = 1.0f;

constexpr float kLightMapFar = 300.0f;

constexpr float kLightMapHeight = 100.0f;

/// `ProjectorRTTLight` is `KSETTINGS_SHADOW_RESOLUTION` square;
/// the high preset's 512 (`CSettings::assignDefault`).
constexpr unsigned kLightMapResolution = 512;

/// The light map's clear colour until a level's rules set LIGHTMAP
/// (0x3dc0c0c1, 24/255).
constexpr float kLightMapDefaultClear = 24.0f / 255.0f;

/// What the light map queues: `CGameClient::renderableQueued`
/// with the shadow flag (`+0x2b0`) clear takes groups 0, 95-99 and 101.
class LightMapRenderables : public Ogre::RenderQueue::RenderableListener {
public:
    bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 group, Ogre::ushort,
                          Ogre::Technique** technique, Ogre::RenderQueue*) override {
        compile_waiting_material(rend, technique);
        return group == Ogre::RENDER_QUEUE_BACKGROUND ||
               (group >= kLightMapGroupFirst && group <= kLightMapGroupLast) ||
               group == kLightMapGroupLast + 2;
    }
};

/// `CGameClient::preRenderTargetUpdate` / `postRenderTargetUpdate`
/// for the light map: its listener in for the update, the quads joined meanwhile; the mask
/// is the viewport's (kVisibilityLightMap). `CLevel::setNonLightStaticGeometryVisible`
/// has nothing to hide.
class LightMapTarget : public Ogre::RenderTargetListener {
public:
    LightMapTarget(Ogre::SceneManager* scene, Ogre::SceneNode* quads,
                   Ogre::RenderQueue::RenderableListener* view)
        : scene_(scene), quads_(quads), view_(view) {}

    // PORT: drawing, the light map's render target refreshed.
    void preRenderTargetUpdate(const Ogre::RenderTargetEvent&) override {
        scene_->getRenderQueue()->setRenderableListener(&renderables_);
        if (!quads_->getParentSceneNode()) scene_->getRootSceneNode()->addChild(quads_);
    }

    void postRenderTargetUpdate(const Ogre::RenderTargetEvent&) override {
        scene_->getRenderQueue()->setRenderableListener(view_);
        if (Ogre::SceneNode* parent = quads_->getParentSceneNode()) parent->removeChild(quads_);
    }

private:
    Ogre::SceneManager* scene_;
    Ogre::SceneNode* quads_;
    Ogre::RenderQueue::RenderableListener* view_;
    LightMapRenderables renderables_;
};

/// `ProjectorCamShadow` (`CGameClient::CGameClient`): FOVy
/// 120, looking down; `CGameClient::refreshLighting`
/// makes it orthographic (`setFrustumExtents(-8, 8, 8, -8)`) for ORTHOGRAPHICSHADOWS
/// (`CLevelTemplateData +0x8b0`). Kept 5 above the focus
/// (`CCameraControl::updateGameCamera`).
constexpr float kShadowMapHalfExtent = 8.0f;

constexpr float kShadowMapFovDegrees = 120.0f;

constexpr float kShadowMapNear = 1.0f;

constexpr float kShadowMapFar = 150.0f;

constexpr float kShadowMapHeight = 5.0f;

/// `ProjectorRTT` is `SHADOW_RESOLUTION` square, cleared to grey 0.5:
/// the receivers double the map, so the clear leaves a surface as it was.
constexpr unsigned kShadowMapResolution = kLightMapResolution;

constexpr float kShadowMapClear = 0.5f;

/// `ProjectorBackgroundQuad` is on group 94 (`setRenderQueueGroup(0x5e)`), after the
/// casters here.
constexpr unsigned kShadowFadeGroup = kParticleGroupBase + 4;

/// `RenderShadow<i>` and its skinned twins (`CGame::createMaterials`):
/// twelve of each.
constexpr int kShadowLevels = 12;

/// What the shadow map queues: `CGameClient::renderableQueued`
/// with the shadow flag (X360 `+0x220`, Linux `+0x2b0`) set: a casting (`+0x1`),
/// visible (`+0x134`) sub-entity drawn with `RenderShadow<floor(opacity * 12) - 1>`, or its
/// skinned twin for its weight count (`+0x12c`), made per bone and weight count.
class ShadowCasters : public Ogre::RenderQueue::RenderableListener {
public:
    explicit ShadowCasters(GpuSkinning* gpu) : gpu_(gpu) {}
    bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 group, Ogre::ushort,
                          Ogre::Technique** technique, Ogre::RenderQueue*) override;

private:
    Ogre::MaterialPtr material(int level, unsigned bones, unsigned weights);

    GpuSkinning* gpu_;
    std::map<std::tuple<int, unsigned, unsigned>, Ogre::MaterialPtr> materials_;
};

/// `CGameClient::preRenderTargetUpdate` / `postRenderTargetUpdate`
/// for the shadow map (kVisibilityShadowMap).
class ShadowMapTarget : public Ogre::RenderTargetListener {
public:
    ShadowMapTarget(Ogre::SceneManager* scene, Ogre::SceneNode* quad,
                    Ogre::RenderQueue::RenderableListener* view, GpuSkinning* gpu)
        : scene_(scene), quad_(quad), view_(view), casters_(gpu) {}

    // PORT: drawing, the shadow map's render target refreshed.
    void preRenderTargetUpdate(const Ogre::RenderTargetEvent&) override {
        scene_->getRenderQueue()->setRenderableListener(&casters_);
        if (!quad_->getParentSceneNode()) scene_->getRootSceneNode()->addChild(quad_);
    }

    void postRenderTargetUpdate(const Ogre::RenderTargetEvent&) override {
        scene_->getRenderQueue()->setRenderableListener(view_);
        if (Ogre::SceneNode* parent = quad_->getParentSceneNode()) parent->removeChild(quad_);
    }

private:
    Ogre::SceneManager* scene_;
    Ogre::SceneNode* quad_;
    Ogre::RenderQueue::RenderableListener* view_;
    ShadowCasters casters_;
};

std::unique_ptr<Ogre::Rectangle2D> light_map_quad(float half, const Ogre::MaterialPtr& material,
                                                  Ogre::uint8 group);
Ogre::MaterialPtr light_map_material(const std::string& name, Ogre::SceneBlendType blend);
void set_light_map_texture(const Ogre::MaterialPtr& material, const std::string& reference);
}  // namespace renderer_detail

tl::core::DungeonManager& loaded_dungeon_manager();
tl::core::DungeonFiles archive_dungeon_files();
bool subject_in_shot(const Ogre::Camera* camera, const Ogre::Entity* subject);
namespace renderer_detail {
std::string biome_key_from_ruleset(const std::string& ruleset);
std::string biome_display_name(const std::string& key);
using tl::core::orientation_bytes;
using tl::core::yaw_of_bytes;
tl::formats::QuestManagerSave quests_to_save(
    const tl::core::QuestLog& log, const std::function<int64_t(const std::string&)>& unit_guid_of);
std::string save_directory_of(const std::string& path);
extern bool keep_loading_screen;
extern bool keep_game_hud;
extern bool keep_inventory;
Ogre::MeshPtr load_gear_mesh(const std::string& path, const std::string& fallback_dir);
Ogre::Vector3 line_point(const Ogre::Vector3 (&p)[4], float t);
}  // namespace renderer_detail
using tl::core::kQuestBannerSeconds;

/// How fast it then fades, in alpha per second (.rodata, `CGameUI::updateIngameUI`),
/// after the hold.
constexpr float kBannerFadePerSecond = 2.0f;

/// The fidget cooldown: `CMonster::idleAI` tests `+0x284` against 10.0.
constexpr float kFidgetCooldown = 10.0f;

/// How often a unit that may fidget does, per second: ten in 1001 per AI tick at 60 Hz.
/// DIVERGENCE (frame rate): a rate, not a per-frame roll.
constexpr float kFidgetPerSecond = 60.0f * 10.0f / 1001.0f;

/// The word every fidget file carries, matched as a substring
/// (archive_animation_stems_matching()).
constexpr const char* kFidgetWord = "fidget";

/// The owner the pet's model is filed under in `spawned`, apart from every
/// spawner's: the model itself gets a spawned unit's id (`pet_node_id`).
constexpr int64_t kPetNodeId = 0x5350000000000000LL - 1;

/// How far to the character's side a new pet is put: the two authored radii.
constexpr float kPetSpawnOffset = 1.6f;

}  // namespace tl::render
