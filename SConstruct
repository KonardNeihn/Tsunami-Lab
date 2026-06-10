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

# keep existing paths
env.Append(CPPPATH=[f"{os.environ['PUGIXML_INCLUDE']}"])
env.Append(LIBPATH=[f"{os.environ['PUGIXML_LIB']}"])
env.Append(LIBS=["pugixml"])

env.Append(CPPPATH=[os.environ["NETCDF_INCLUDE"]])
env.Append(LIBPATH=[os.environ["NETCDF_LIB"]])
env.Append(LIBS=["netcdf"])

# generate help message
Help( vars.GenerateHelpText( env ) )

# add default flags
env.Append( CXXFLAGS = [ '-std=c++17',
                         '-Wall',
                         '-Wextra',
                         '-Wpedantic' ] )

# 3. FLEXIBLE OPTIMIERUNG EINBAUEN
if 'debug' in env['mode']:
  env.Append( CXXFLAGS = [ '-g', '-O0' ] )
else:
  # Statt hart '-O2' nutzen wir jetzt die Variable aus 'opt'
  env.Append( CXXFLAGS = [ f"-{env['opt']}" ] )
  
  # NEU: Falls native=yes gewählt wurde, Flag anhängen
  if env['native'] == 'yes':
      env.Append( CXXFLAGS = [ '-march=native' ] )

# add sanitizers (Bleibt unverändert)
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

env.Program( target = 'build/tsunami_lab', source = sources + standalone )
env.Program( target = 'build/tests', source = sources + tests )

print("SOURCES:", sources)