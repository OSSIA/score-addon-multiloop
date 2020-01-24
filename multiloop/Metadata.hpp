#pragma once
#include <Process/ProcessMetadata.hpp>

namespace multiloop
{
class Model;
}

PROCESS_METADATA(
    , multiloop::Model, "cc5dea6a-cd77-44aa-8e75-9a2c9a331dd2",
    "multiloop",                                   // Internal name
    "multiloop",                                   // Pretty name
    Process::ProcessCategory::Other,              // Category
    "Other",                                      // Category
    "Description",                                // Description
    "Author",                                     // Author
    (QStringList{"Put", "Your", "Tags", "Here"}), // Tags
    {},                                           // Inputs
    {},                                           // Outputs
    Process::ProcessFlags::SupportsAll            // Flags
)
