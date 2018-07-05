find_program (Seastar_DOXYGEN_EXECUTABLE doxygen)
find_program (Seastar_PANDOC_EXECUTABLE  pandoc)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  @ONLY)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml
  COPYONLY)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html
  DEPENDS
    seastar
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml
  COMMAND ${Seastar_DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_target (doc_html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tutorial.html
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.md
  COMMAND
    ${Seastar_PANDOC_EXECUTABLE}
    --self-contained
    --smart
    --toc
    -c ${CMAKE_CURRENT_SOURCE_DIR}/template.css
    --chapters
    --number-sections
    -f markdown_github+pandoc_title_block+implicit_header_references
    --highlight-style tango
    ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.md
    -o ${CMAKE_CURRENT_BINARY_DIR}/tutorial.html)

add_custom_target (doc_tutorial_html
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tutorial.html)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/htmlsplit.py
  ${CMAKE_CURRENT_BINARY_DIR}/htmlsplit.py
  COPYONLY)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tutorial-split
  DEPENDS
    doc_tutorial_html
    ${CMAKE_CURRENT_BINARY_DIR}/htmlsplit.py
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/tutorial-split
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/htmlsplit.py)

add_custom_target (doc_tutorial_html_split
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tutorial-split)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tutorial.pdf
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.md
  COMMAND
    ${Seastar_PANDOC_EXECUTABLE}
    -f markdown_github+pandoc_title_block+implicit_header_references
    --highlight-style tango
    --template=${CMAKE_CURRENT_SOURCE_DIR}/template.tex
    ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.md
    -o ${CMAKE_CURRENT_BINARY_DIR}/tutorial.pdf)

add_custom_target (doc_tutorial_pdf
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tutorial.pdf)

# Logical target for all documentation.
add_custom_target (docs
  DEPENDS
    doc_html
    doc_tutorial_html
    doc_tutorial_html_split
    doc_tutorial_pdf)
