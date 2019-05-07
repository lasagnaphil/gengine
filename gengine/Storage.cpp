//
// Created by lasagnaphil on 19. 3. 7.
//

#include "Storage.h"
#include "Transform.h"
#include "Shader.h"
#include "Texture.h"
#include "Image.h"
#include "Material.h"
#include "Mesh.h"
#include "LineMesh.h"

#define LIST_OF_VARIABLES \
    GLOBAL_TYPE(Transform, 1, 8) \
    GLOBAL_TYPE(Shader, 2, 8) \
    GLOBAL_TYPE(Texture, 3, 8) \
    GLOBAL_TYPE(Image, 4, 8) \
    GLOBAL_TYPE(Material, 5, 8) \
    GLOBAL_TYPE(Mesh, 6, 8) \
    GLOBAL_TYPE(LineMaterial, 7, 8) \
    GLOBAL_TYPE(LineMesh, 8, 8)

#define GLOBAL_TYPE(__Type, __id, __capacity) \
template <> uint16_t TypeRegistry::getID<__Type>() { return __id; } \
template <> __Type *Ref<__Type>::operator->() const { return Resources::get(*this); } \
template <> __Type &Ref<__Type>::operator*() const { return *Resources::get(*this); } \
template <> __Type *Ref<__Type>::get() { return Resources::get(*this); } \
template <> __Type *Ref<__Type>::tryGet() { return Resources::tryGet(*this); } \
template <> void Ref<__Type>::release() { Resources::release(*this); } \
static Storage<__Type> __Type##Storage = {__capacity}; \
template <> Storage<__Type>& Resources::getStorage() { return __Type##Storage; }

LIST_OF_VARIABLES
#undef GLOBAL_TYPE
