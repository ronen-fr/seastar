#
# This file is open source software, licensed to you under the terms
# of the Apache License, Version 2.0 (the "License").  See the NOTICE file
# distributed with this work for additional information regarding copyright
# ownership.  You may not use this file except in compliance with the License.
#
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

#
# Copyright (C) 2018 Scylladb, Ltd.
#

find_path (numa_INCLUDE_DIR
  NAMES numa.h)

find_library (numa_LIBRARY
  NAMES numa)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (numa
  FOUND_VAR numa_FOUND
  REQUIRED_VARS
    numa_INCLUDE_DIR
    numa_LIBRARY)

if (numa_FOUND)
  set (numa_INCLUDE_DIRS ${numa_INCLUDE_DIR})
endif ()

if (numa_FOUND AND NOT (TARGET numa::numa))
  add_library (numa::numa UNKNOWN IMPORTED)

  set_target_properties (numa::numa
    PROPERTIES
      IMPORTED_LOCATION ${numa_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${numa_INCLUDE_DIR})
endif ()

mark_as_advanced (
  numa_INCLUDE_DIR
  numa_LIBRARY)
