#include "precompiled.h"

#include "SaveData.h"
#include "../util/Debug_MemoryManager.h"

namespace game
{
    SaveData::SaveData(int id, int size, const uint8_t *a_data, int childAmount,
                       GameObject **children)
    {
        this->id = id;
        this->size = size;
        if (a_data != NULL) {
            this->data = new uint8_t[size];
            memcpy(this->data, a_data, size);
        } else {
            this->data = NULL;
        }
        if (childAmount > 0) {
            if (children == NULL) abort();
            this->children = new GameObject *[childAmount];
            for (int i = 0; i < childAmount; i++) {
                this->children[i] = children[i];
            }
        } else {
            if (children != NULL) abort();
        }
    }

    SaveData::~SaveData()
    {
        if (data != NULL) delete[] data;
        data = NULL;
        if (children != NULL) delete[] children;
        children = NULL;
    }

}
