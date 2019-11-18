//
// Created by lasagnaphil on 19. 3. 7.
//

#include "GenAllocator.h"
#include "Transform.h"
#include "Shader.h"
#include "Texture.h"
#include "Image.h"
#include "PhongRenderer.h"
#include "PBRenderer.h"
#include "Mesh.h"
#include "LineMesh.h"

#ifdef USE_SHARED_PTR
#else
#define LIST_OF_VARIABLES \
    GLOBAL_TYPE(Transform, 1, 256) \
    GLOBAL_TYPE(Shader, 2, 16) \
    GLOBAL_TYPE(Texture, 3, 32) \
    GLOBAL_TYPE(Image, 4, 32) \
    GLOBAL_TYPE(Mesh, 5, 8) \
    GLOBAL_TYPE(LineMesh, 6, 8) \
    GLOBAL_TYPE(PhongMaterial, 7, 8) \
    GLOBAL_TYPE(LineMaterial, 8, 8) \
    GLOBAL_TYPE(PBRMaterial, 9, 8)

#define GLOBAL_TYPE(__Type, __id, __capacity) \
template <> uint16_t TypeRegistry::getID<__Type>() { return __id; } \
template <> __Type *Ref<__Type>::operator->() const { return Resources::get(*this); } \
template <> __Type &Ref<__Type>::operator*() const { return *Resources::get(*this); } \
template <> __Type *Ref<__Type>::get() { return Resources::get(*this); } \
template <> __Type *Ref<__Type>::tryGet() { return Resources::tryGet(*this); } \
template <> void Ref<__Type>::release() { Resources::release(*this); } \
static GenAllocator<__Type> __Type##Storage; \
template <> GenAllocator<__Type>& Resources::getStorage() { return __Type##Storage; }

LIST_OF_VARIABLES
#undef GLOBAL_TYPE

#define GLOBAL_TYPE(__Type, __id, __capacity) \
__Type##Storage = GenAllocator<__Type>(__capacity);

Resources::constructor::constructor() {
    // LIST_OF_VARIABLES
}

#undef GLOBAL_TYPE

#endif
