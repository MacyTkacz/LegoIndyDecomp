#pragma once

#include "types.h"

class FileIOManager;

#define FILERESOURCETYPESWITCH(id) \
    switch( FileIOManager::GetResourceType(id) ) {

#define CASE_FILEBUFFERCONTAINER() \
    case FileResourceType::FILEBUFFERCONTAINER: {

#define CASE_FILEHANDLECONTAINER() \
    case FileResourceType::FILEHANDLECONTAINER: {

#define CASE_FILEPOINTERINFO() \
    case FileResourceType::FILEPOINTERINFO: {