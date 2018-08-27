APPNAME = 'ca-sandbox'
VERSION = '0.1.0'

top = '.'
out = 'build'


def configure(conf):

  from subprocess import check_output
  sdl_flags = check_output(['sdl2-config', '--cflags']).split()
  sdl_libs  = check_output(['sdl2-config', '--libs'  ]).split()

  conf.load('clang++')
  conf.env.append_value('CXXFLAGS', ['-Werror', '-std=c++14'] + sdl_flags)
  conf.env.append_value('INCLUDES', ['./include', './include/imgui'])
  #conf.env.append_value('LINKFLAGS', ['-v'])

  conf.env.LIB_LOADER = ['GLEW', 'GL', 'GLU', 'dl']
  conf.env.LINKFLAGS_LOADER = ['-Wl,-export-dynamic,--no-undefined,-rpath,./'] + sdl_libs

  base_env = conf.env.derive()

  conf.setenv('debug', env=base_env)
  conf.env.append_value('CXXFLAGS', ['-O0', '-ggdb'])
  conf.env.append_value('DEFINES', ['_DEBUG'])

  conf.setenv('release', env=base_env)
  conf.env.append_value('CXXFLAGS', ['-Ofast'])


def build(bld):

  if not bld.variant:
    bld.fatal('Must use `{0}_debug` or `{0}_release`'.format(bld.cmd))

  bld.shlib(source=bld.path.ant_glob('src/*/**/*.cpp', excl='src/engine/*'),
            target=APPNAME)

  bld.program(source=bld.path.ant_glob(['src/*.cpp', 'src/engine/**/*.cpp']),
              target='loader',
              use='LOADER')

  bld(rule='ln -s {} {}'.format(bld.path.find_node('cells').abspath(), 'cells'),
      source=bld.path.find_node('cells'),
      target='cells')

  import waflib.extras.buildcopy
  bld(features = 'buildcopy',
      buildcopy_source = bld.path.ant_glob('fonts/**/*'))
  bld(features = 'buildcopy',
      buildcopy_source = bld.path.ant_glob('shaders/**/*'))


def init(ctx):
  from waflib.Build import BuildContext, StepContext, CleanContext, InstallContext, UninstallContext, ListContext

  for x in 'debug release'.split():
    for y in (BuildContext, StepContext, CleanContext, InstallContext, UninstallContext, ListContext):
      name = y.__name__.replace('Context','').lower()
      class tmp(y):
        cmd = name + '_' + x
        variant = x
