/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class CodeBlocksProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameCodeBlocks()      { return "Code::Blocks project"; }
    static const char* getValueTreeTypeName()   { return "CODEBLOCKS"; }

    static CodeBlocksProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new CodeBlocksProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    CodeBlocksProjectExporter (Project& p, const ValueTree& t)   : ProjectExporter (p, t)
    {
        name = getNameCodeBlocks();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "CodeBlocks";
    }

    //==============================================================================
    bool canLaunchProject() override                 { return false; }
    bool launchProject() override                    { return false; }
    bool isCodeBlocks() const override               { return true; }
    bool isWindows() const override                  { return true; }
    bool usesMMFiles() const override                { return false; }
    bool canCopeWithDuplicateFiles() override        { return false; }

    void createExporterProperties (PropertyListBuilder&) override
    {
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        const File cbpFile (getTargetFolder().getChildFile (project.getProjectFilenameRoot())
                                             .withFileExtension (".cbp"));

        XmlElement xml ("CodeBlocks_project_file");
        addVersion (xml);
        createProject (*xml.createNewChildElement ("Project"));
        writeXmlOrThrow (xml, cbpFile, "UTF-8", 10);
    }

private:
    //==============================================================================
    class CodeBlocksBuildConfiguration  : public BuildConfiguration
    {
    public:
        CodeBlocksBuildConfiguration (Project& p, const ValueTree& settings)
            : BuildConfiguration (p, settings)
        {
        }

        void createConfigProperties (PropertyListBuilder&)
        {
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new CodeBlocksBuildConfiguration (project, tree);
    }

    //==============================================================================
    void addVersion (XmlElement& xml) const
    {
        XmlElement* fileVersion = xml.createNewChildElement ("FileVersion");
        fileVersion->setAttribute ("major", 1);
        fileVersion->setAttribute ("minor", 6);
    }

    void addOptions (XmlElement& xml) const
    {
        xml.createNewChildElement ("Option")->setAttribute ("title", project.getTitle());
        xml.createNewChildElement ("Option")->setAttribute ("pch_mode", 2);
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");
    }

    static StringArray cleanArray (StringArray s)
    {
        s.trim();
        s.removeDuplicates (false);
        s.removeEmptyStrings (true);
        return s;
    }

    StringArray getDefines (const BuildConfiguration& config) const
    {
        StringPairArray defines;
        defines.set ("__MINGW__", "1");
        defines.set ("__MINGW_EXTENSION", String::empty);
        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

        StringArray defs;
        for (int i = 0; i < defines.size(); ++i)
            defs.add (defines.getAllKeys()[i] + "=" + defines.getAllValues()[i]);

        return cleanArray (defs);
    }

    StringArray getCompilerFlags (const BuildConfiguration& config) const
    {
        StringArray flags;
        flags.add ("-O" + config.getGCCOptimisationFlag());
        flags.add ("-std=gnu++0x");
        flags.add ("-mstackrealign");

        if (config.isDebug())
            flags.add ("-g");

        flags.addTokens (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim(),
                         " \n", "\"'");

        {
            const StringArray defines (getDefines (config));

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines[i]);

                if (! def.containsChar ('='))
                    def << '=';

                flags.add ("-D" + def);
            }
        }

        return cleanArray (flags);
    }

    StringArray getLinkerFlags (const BuildConfiguration& config) const
    {
        StringArray flags;

        if (! config.isDebug())
            flags.add ("-s");

        flags.addTokens (replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim(),
                         " \n", "\"'");

        return cleanArray (flags);
    }

    StringArray getIncludePaths (const BuildConfiguration& config) const
    {
        StringArray paths;
        paths.add (".");
        paths.add (RelativePath (project.getGeneratedCodeFolder(),
                                 getTargetFolder(), RelativePath::buildTargetFolder).toWindowsStyle());

        paths.addArray (config.getHeaderSearchPaths());
        return cleanArray (paths);
    }

    static int getTypeIndex (const ProjectType& type)
    {
        if (type.isGUIApplication()) return 0;
        if (type.isCommandLineApp()) return 1;
        if (type.isStaticLibrary())  return 2;
        if (type.isDynamicLibrary()) return 3;
        if (type.isAudioPlugin())    return 3;
        return 0;
    }

    void createBuildTarget (XmlElement& xml, const BuildConfiguration& config) const
    {
        xml.setAttribute ("title", config.getName());

        {
            XmlElement* output = xml.createNewChildElement ("Option");

            String outputPath;
            if (config.getTargetBinaryRelativePathString().isNotEmpty())
            {
                RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
                binaryPath = binaryPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder);
                outputPath = config.getTargetBinaryRelativePathString();
            }
            else
            {
                outputPath ="bin/" + File::createLegalFileName (config.getName().trim());
            }

            output->setAttribute ("output", outputPath + "/" + replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

            output->setAttribute ("prefix_auto", 1);
            output->setAttribute ("extension_auto", 1);
        }

        xml.createNewChildElement ("Option")
             ->setAttribute ("object_output", "obj/" + File::createLegalFileName (config.getName().trim()));

        xml.createNewChildElement ("Option")->setAttribute ("type", getTypeIndex (project.getProjectType()));
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");

        {
            XmlElement* const compiler = xml.createNewChildElement ("Compiler");

            {
                const StringArray compilerFlags (getCompilerFlags (config));

                for (int i = 0; i < compilerFlags.size(); ++i)
                    setAddOption (*compiler, "option", compilerFlags[i]);
            }

            {
                const StringArray includePaths (getIncludePaths (config));

                for (int i = 0; i < includePaths.size(); ++i)
                    setAddOption (*compiler, "directory", includePaths[i]);
            }
        }

        {
            XmlElement* const linker = xml.createNewChildElement ("Linker");

            const StringArray linkerFlags (getLinkerFlags (config));
            for (int i = 0; i < linkerFlags.size(); ++i)
                setAddOption (*linker, "option", linkerFlags[i]);

            for (int i = 0; i < mingwLibs.size(); ++i)
                setAddOption (*linker, "library", mingwLibs[i]);

            const StringArray librarySearchPaths (config.getLibrarySearchPaths());
            for (int i = 0; i < librarySearchPaths.size(); ++i)
                setAddOption (*linker, "directory", replacePreprocessorDefs (getAllPreprocessorDefs(), librarySearchPaths[i]));
        }
    }

    void addBuild (XmlElement& xml) const
    {
        XmlElement* const build = xml.createNewChildElement ("Build");

        for (ConstConfigIterator config (*this); config.next();)
            createBuildTarget (*build->createNewChildElement ("Target"), *config);
    }

    void addProjectCompilerOptions (XmlElement& xml) const
    {
        XmlElement* const compiler = xml.createNewChildElement ("Compiler");
        setAddOption (*compiler, "option", "-Wall");
        setAddOption (*compiler, "option", "-Wno-strict-aliasing");
        setAddOption (*compiler, "option", "-Wno-strict-overflow");
    }

    void addProjectLinkerOptions (XmlElement& xml) const
    {
        XmlElement* const linker = xml.createNewChildElement ("Linker");

        static const char* defaultLibs[] = { "gdi32", "user32", "kernel32", "comctl32" };

        StringArray libs (defaultLibs, numElementsInArray (defaultLibs));
        libs.addTokens (getExternalLibrariesString(), ";\n", "\"'");

        libs = cleanArray (libs);

        for (int i = 0; i < libs.size(); ++i)
            setAddOption (*linker, "library", replacePreprocessorDefs (getAllPreprocessorDefs(), libs[i]));
    }

    void addCompileUnits (const Project::Item& projectItem, XmlElement& xml) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addCompileUnits (projectItem.getChild(i), xml);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath file (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            XmlElement* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", file.toUnixStyle());

            if (! projectItem.shouldBeCompiled())
            {
                unit->createNewChildElement("Option")->setAttribute ("compile", 0);
                unit->createNewChildElement("Option")->setAttribute ("link", 0);
            }
        }
    }

    void addCompileUnits (XmlElement& xml) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
            addCompileUnits (getAllGroups().getReference(i), xml);
    }

    void createProject (XmlElement& xml) const
    {
        addOptions (xml);
        addBuild (xml);
        addProjectCompilerOptions (xml);
        addProjectLinkerOptions (xml);
        addCompileUnits (xml);
    }

    void setAddOption (XmlElement& xml, const String& nm, const String& value) const
    {
        xml.createNewChildElement ("Add")->setAttribute (nm, value);
    }

    JUCE_DECLARE_NON_COPYABLE (CodeBlocksProjectExporter)
};
