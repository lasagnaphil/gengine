//
// Created by lasagnaphil on 20. 2. 7..
//

#ifndef DEEPMIMIC_RTTR_MACRO_H
#define DEEPMIMIC_RTTR_MACRO_H

#include <rttr/registration.h>

#define RTTR_PROP(Type, field) \
    .property(#field, &Type::field)

#define RTTR_SIMPLE(Type, Fields) \
    rttr::registration::class_<Type>(#Type) \
        .constructor<>() \
        Fields(Type) \
        ; \

#endif //DEEPMIMIC_RTTR_MACRO_H
