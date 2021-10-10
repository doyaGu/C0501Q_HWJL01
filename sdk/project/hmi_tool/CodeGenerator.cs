using System;
using System.Collections.Generic;
using System.IO;

namespace GUIDesigner
{
    public class CodeGenerator
    {
        public CodeGenerator()
	    {
	    }

        public void GenerateFileTable(string functionTablePath, HashSet<string> functions)
        {
            using (StreamWriter outputFile = new StreamWriter(functionTablePath))
            {
                outputFile.WriteLine("#include \"ite/itu.h\"");
                outputFile.WriteLine();

                foreach (var funcName in functions)
                {
                    if (!string.IsNullOrEmpty(funcName))
                    {
                        outputFile.Write("extern bool ");
                        outputFile.Write(funcName);
                        outputFile.WriteLine("(ITUWidget* widget, char* param);");
                    }
                }
                outputFile.WriteLine();

                outputFile.WriteLine("ITUActionFunction actionFunctions[] =");
                outputFile.WriteLine("{");

                foreach (var funcName in functions)
                {
                    if (!string.IsNullOrEmpty(funcName))
                    {
                        outputFile.Write("    \"");
                        outputFile.Write(funcName);
                        outputFile.Write("\", ");
                        outputFile.Write(funcName);
                        outputFile.WriteLine(",");
                    }
                }

                outputFile.WriteLine("    NULL, NULL");
                outputFile.WriteLine("};");
                outputFile.Close();
            }
        }

        public void GenerateLayerCode(string layerPath, HashSet<string> functions)
        {
            using (StreamWriter outputFile = new StreamWriter(layerPath))
            {
                outputFile.WriteLine("#include <assert.h>");
                outputFile.WriteLine();

                foreach (var funcName in functions)
                {
                    if (!string.IsNullOrEmpty(funcName))
                    {
                        outputFile.Write("bool ");
                        outputFile.Write(funcName);
                        outputFile.WriteLine("(ITUWidget* widget, char* param)");
                        outputFile.WriteLine("{");
                        outputFile.WriteLine("    return true;");
                        outputFile.WriteLine("}");
                        outputFile.WriteLine();
                    }
                }
                outputFile.Close();
            }
        }

        public int FindFileLine(string filePath, string funcName)
        {
            string pattern = "bool " + funcName + "(ITUWidget*";

            using (StreamReader inputFile = new StreamReader(filePath))
            {
                int line = 1;
                string s = inputFile.ReadLine();
                while (s != null)
                {
                    if (s.IndexOf(pattern) >= 0)
                    {
                        inputFile.Close();
                        return line;
                    }
                    s = inputFile.ReadLine();
                    line++;
                }
                inputFile.Close();
            }
            return 0;
        }

        public void AppendFunction(string filePath, string funcName)
        {
            using (StreamWriter outputFile = File.AppendText(filePath))
            {
                outputFile.WriteLine();
                outputFile.Write("bool ");
                outputFile.Write(funcName);
                outputFile.WriteLine("(ITUWidget* widget, char* param)");
                outputFile.WriteLine("{");
                outputFile.WriteLine("    return true;");
                outputFile.WriteLine("}");
                outputFile.WriteLine();
                outputFile.Close();
            }
        }

        public static void InvokeVisualStudio(string funcName)
        {
            LayerWidget layerWidget = ITU.layerWidget as LayerWidget;
            string filePath = ITU.projectPath + "\\" + layerWidget.CFileName;

            EnvDTE80.DTE2 dte2 = null;
            try
            {
                dte2 = (EnvDTE80.DTE2)System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE.12.0");
            }
            catch (Exception)
            {
                try
                {
                    dte2 = (EnvDTE80.DTE2)System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE.9.0");
                }
                catch (Exception)
                {
                }
            }

            if (dte2 != null)
            {
                FileInfo fInfo = new FileInfo(filePath);
                if (fInfo.Exists)
                {
                    CodeGenerator generator = new CodeGenerator();
                    int line = generator.FindFileLine(filePath, funcName);
                    if (line > 0)
                    {
                        dte2.MainWindow.Activate();
                        EnvDTE.Window w = dte2.ItemOperations.OpenFile(filePath, "{7651A703-06E5-11D1-8EBD-00A0C90F26EA}");
                        ((EnvDTE.TextSelection)dte2.ActiveDocument.Selection).GotoLine(line, true);
                    }
                    else
                    {
                        generator.AppendFunction(filePath, funcName);
                        line = generator.FindFileLine(filePath, funcName);
                        if (line > 0)
                        {
                            dte2.MainWindow.Activate();
                            EnvDTE.Window w = dte2.ItemOperations.OpenFile(filePath, "{7651A703-06E5-11D1-8EBD-00A0C90F26EA}");
                            ((EnvDTE.TextSelection)dte2.ActiveDocument.Selection).GotoLine(line, true);
                        }
                    }
                }
            }
        }

        public void GenerateLayerCMake(string layerCMakePath, HashSet<string> layers)
        {
            using (StreamWriter outputFile = new StreamWriter(layerCMakePath))
            {
                outputFile.WriteLine("add_executable(${CMAKE_PROJECT_NAME}");

                foreach (var layerCName in layers)
                {
                    if (!string.IsNullOrEmpty(layerCName))
                    {
                        outputFile.Write("    ");
                        outputFile.Write(layerCName);
                        outputFile.WriteLine();
                    }
                }
                outputFile.WriteLine(")");
                outputFile.WriteLine();
                outputFile.Close();
            }
        }
    }
}