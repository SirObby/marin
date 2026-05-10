project('marin', 'cpp',
  version : '1.0',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

dpp_dep = dependency('dpp')
pq_dep = dependency('libpq')
pqxx_dep = dependency('libpqxx')

inc = include_directories('src')

src_files = [
  'src/main.cpp',
  'src/commands/ping.cpp',
  'src/commands/info.cpp',
  'src/commands/profile.cpp',
  'src/commands/search.cpp',
  'src/commands/character.cpp',
  'src/commands/manga.cpp',
  'src/commands/settings.cpp',
  'src/commands/delete_data.cpp',
  'src/listeners/on_select_click.cpp',
  'src/listeners/on_button_click.cpp',
  'src/listeners/on_modal_submit.cpp',
  'src/shared/render.cpp',
  'src/shared/anilist.cpp',
  'src/shared/database.cpp',
  'src/shared/i18n.cpp'
]

marin_exe = executable('marin',
  src_files,
  include_directories: inc,
  dependencies: [dpp_dep, pqxx_dep, pq_dep],
  install : true)

# Simple test suite
test_src = [
  'tests/test_main.cpp',
  'tests/test_i18n.cpp',
  'src/shared/i18n.cpp'
]

test_exe = executable('marin_tests',
  test_src,
  include_directories: inc,
  dependencies: [dpp_dep]
)

test('marin tests', test_exe)
