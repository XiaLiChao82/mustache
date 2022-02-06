#include "archetype_operation_helper.hpp"
#include <mustache/ecs/entity.hpp>
#include <mustache/ecs/component_factory.hpp>

using namespace mustache;

ArchetypeOperationHelper::ArchetypeOperationHelper(MemoryManager& memory_manager, const ComponentIdMask& mask):
        component_id_to_component_index{memory_manager},
        component_index_to_component_id {memory_manager},
        insert{memory_manager},
        create_with_value{memory_manager},
        destroy{memory_manager},
        external_move{memory_manager},
        internal_move{memory_manager} {

    for (auto component_id : mask.items()) {
        component_index_to_component_id.push_back(component_id);
    }

    ComponentIndex component_index = ComponentIndex::make(0);

    for (auto component_id : component_index_to_component_id) {

        if (!component_id_to_component_index.has(component_id)) {
            component_id_to_component_index.resize(component_id.next().toInt());
        }
        component_id_to_component_index[component_id] = component_index;
        const auto& info = ComponentFactory::componentInfo(component_id);

        if (info.functions.create) {
            insert.push_back(InsertInfo {
                    info.functions.create,
                    component_index
            });
        } else if (!info.default_value.empty()) {
            create_with_value.push_back(CreateWithValueInfo {
                    info.default_value.data(),
                    info.default_value.size(),
                    component_index
            });
        }
        if (info.functions.destroy) {
            destroy.push_back(DestroyInfo {
                    info.functions.destroy,
                    component_index
            });
        }
        if (info.functions.move) {
            internal_move.push_back(InternalMoveInfo {
                    info.functions.move,
                    info.size
            });
        }
        ExternalMoveInfo& external_move_info = external_move.emplace_back();
        external_move_info.constructor_ptr = info.functions.create;
        external_move_info.move_ptr = info.functions.move_constructor;
        external_move_info.id = component_id;
        external_move_info.size = info.size;
        external_move_info.default_data = info.default_value.empty() ? nullptr : info.default_value.data();

        ++component_index;
    }
}
