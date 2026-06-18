##
# @author Alexander Breuer (alex.breuer AT uni-jena.de)
#
# @section DESCRIPTION
# Entry-point for builds.
##
import SCons
import os

print( '####################################' )
print( '### Tsunami Lab                  ###' )
print( '###                              ###' )
print( '### https://scalable.uni-jena.de ###' )
print( '####################################' )
print()
print('running build script')

# extended configuration
vars = Variables()

# new complex variables
vars.AddVariables(
  EnumVariable( 'mode',
                'compile modes, option \'san\' enables address and undefined behavior sanitizers',
                'release',
                allowed_values=('release', 'debug', 'release+san', 'debug+san' )
              ),
  EnumVariable( 'opt',
                'optimisationlevel for performance-tests',
                'O2',
                allowed_values=('O2', 'O3', 'Ofast')
              ),
  EnumVariable( 'native',
                'use hardware-specific optimisation (-march=native)',
                'no',
                allowed_values=('yes', 'no')
              )
)

# include new variables with .Add() 
vars.Add('cxx', 'Use g++ or clang++', None)

# exit in the case of unknown variables
if vars.UnknownVariables():
  print( "build configuration corrupted, don't know what to do with: " + str(vars.UnknownVariables().keys()) )
  exit(1)

# create environment
env = Environment( variables = vars )

# add environment variables for compilers
env['ENV'] = os.environ

# COMPILER-SELECTION 
# choose compiler from CXX or otherwise use g++ 
if env.get('cxx'):
    env['CXX'] = env['cxx']
else:
    env['CXX'] = os.environ.get('CXX', 'g++')

# support for pugixml
#env.Append(CPPPATH=['#thirdparty/pugixml'])

# reading paths with fallbacks
#netcdf_inc = os.environ.get("NETCDF_INCLUDE", "/usr/include")
#netcdf_lib = os.environ.get("NETCDF_LIB", "/usr/lib64")

# keep existing paths
env.Append(CPPPATH=[f"{os.environ['PUGIXML_INCLUDE']}"])
env.Append(LIBPATH=[f"{os.environ['PUGIXML_LIB']}"])
env.Append(LIBS=["pugixml"])

#env.Append(CPPPATH=[netcdf_inc])
#env.Append(LIBPATH=[netcdf_lib])
env.Append(LIBS=["netcdf"])
env.Append(LIBS=['stdc++fs'])

#env.ParseConfig("pkg-config --cflags --libs pugixml")
#env.ParseConfig("pkg-config --cflags --libs netcdf"

# generate help message
Help( vars.GenerateHelpText( env ) )

# add default flags
env.Append( CXXFLAGS = [ '-std=c++17',
                         '-Wall',
                         '-Wextra',
                         '-Wpedantic',
                         '-fopenmp' ] )


env.Append( LINKFLAGS = [ '-fopenmp' ] )

# flexible optimisation
if 'debug' in env['mode']:
  env.Append( CXXFLAGS = [ '-g', '-Og' ] )
else:
  # instead of -O2, use given variable
  env.Append( CXXFLAGS = [ f"-{env['opt']}" ] )
  
  # Clang-Optimisationreports
  #env.Append( CXXFLAGS = [
  #  '-Rpass=.*',
  #  '-Rpass-missed=.*',
  #  '-Rpass-analysis=.*'
  #])
  
  # append flag if native=yes 
  if env['native'] == 'yes':
      env.Append( CXXFLAGS = [ '-march=native' ] )

# add sanitizers
if 'san' in env['mode']:
  env.Append( CXXFLAGS =  [ '-g',
                            '-fsanitize=float-divide-by-zero',
                            '-fsanitize=bounds',
                            '-fsanitize=address',
                            '-fsanitize=undefined',
                            '-fno-omit-frame-pointer' ] )
  env.Append( LINKFLAGS = [ '-g',
                            '-fsanitize=address',
                            '-fsanitize=undefined' ] )
else:
  # comment out -Werror with -Ofast if it does not work otherwise
  env.Append( CXXFLAGS = [ '-Werror' ] )

# Info to see current setup
print( f"--> COMPILING WITH: {env['CXX']} | OPT: -{env['opt']} | NATIVE: {env['native']}" )

# add Catch2
env.Append( CXXFLAGS = [ '-isystem', 'submodules/Catch2/single_include' ] )

# get source files
VariantDir( variant_dir = 'build/src', src_dir = 'src' )

sources, tests, standalone = SConscript(
    'build/src/SConscript',
    exports='env'
)

# RPATH für das Cluster setzen, damit die richtige libstdc++ fest im Binary verankert ist
#if os.environ.get('CMPLR_ROOT'):
    # Pfad zur Intel-eigenen libstdc++ beziehungsweise GCC-Bibliothek
    #gcc_lib64 = "/cluster/spack/v0.19/opt/spack/linux-almalinux8-x86_64_v3/gcc-8.5.0/gcc-12.2.0-ithezm6tiwdinin3jketpzd5wfvdtjny/lib64"
    #env.Append(LINKFLAGS=[f"-Wl,-rpath,{gcc_lib64}", f"-Wl,-rpath,{os.environ['CMPLR_ROOT']}/linux/lib"])

# additional dependecies for pugixml
env.Program( target = 'build/tsunami_lab', source = sources + standalone) # + ['thirdparty/pugixml/pugixml.cpp'] )
env.Program( target = 'build/tests', source = sources + tests) # + ['thirdparty/pugixml/pugixml.cpp'] )

print("SOURCES:", sources)
