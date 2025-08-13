using System.Diagnostics;
using System.Text;

namespace MilkwaveRemote.Data {
  public class Shader {
    private string indent = "";
    public StringBuilder ConversionErrors = new StringBuilder();

    public string ConvertGLSLtoHLSL(string inp) {
      string result = "";
      StringBuilder sb = new StringBuilder();
      ConversionErrors = new StringBuilder();
      try {
        inp = inp.Replace("vec2", "float2").Replace("vec3", "float3").Replace("vec4", "float4");
        inp = inp.Replace("fract (", "fract(").Replace("mod (", "mod(").Replace("mix (", "mix (");
        inp = inp.Replace("fract(", "frac(").Replace("mod(", "mod_conv(").Replace("mix(", "lerp(");
        inp = ReplaceVarName("time", "time_conv", inp);
        inp = inp.Replace("iTime", "time").Replace("iResolution", "uv");
        inp = inp.Replace("iFrame", "frame");
        inp = inp.Replace("texture(", "tex2D(");
        inp = inp.Replace("void mainImage(", "mainImage(");

        int indexMainImage = inp.IndexOf("mainImage(");

        string inpHeader = "";
        string inpMain = "";
        string inpFooter = "";
        string retVarName = "";

        if (indexMainImage == -1) {
          // no mainImage function, we'll just wrap the full input into a shader_body
          inpMain = inp + Environment.NewLine + "}"; // opening bracket is supplied below
        } else {
          int indexMainImageMethodClosingBracket = FindClosingBracketIndex(inp.Substring(indexMainImage), '{', '}', 0);

          inpHeader = inp.Substring(0, indexMainImage);

          inpHeader = StripCommentsAndBlankLines(inpHeader);

          inpMain = inp.Substring(indexMainImage, indexMainImageMethodClosingBracket + 1);
          inpFooter = "";

          int footerIndex = indexMainImage + indexMainImageMethodClosingBracket + 1;
          if (inp.Length > footerIndex) {
            inpFooter = inp.Substring(footerIndex);
          }
          inpHeader += inpFooter;
        }

        inpHeader = ReplaceVarName("uv", "uv_conv", inpHeader);

        if (inp.Contains("mod_conv(")) {
          inpHeader = AddHelperFunctionsMod(inpHeader);
        }
        if (inp.Contains("lessthan", StringComparison.InvariantCultureIgnoreCase)) {
          inpHeader = AddHelperFunctionsLessThan(inpHeader);
        }

        StringBuilder sbHeader = new StringBuilder();
        sbHeader.AppendLine();
        sbHeader.AppendLine("shader_body {");

        if (indexMainImage > -1) {
          // if we have a mainImage function, convert its arguments

          int indexMainImageArgsOpeningBracket = inpMain.IndexOf("(") + 1;
          int indexMainImageArgsClosingBracket = FindClosingBracketIndex(inpMain.Substring(indexMainImageArgsOpeningBracket), '(', ')', 1);

          string mainImageArgsString = inpMain.Substring(indexMainImageArgsOpeningBracket, indexMainImageArgsClosingBracket);
          string[] mainImageArgs = mainImageArgsString.Split(",");
          for (int i = 0; i < mainImageArgs.Length; i++) {
            string arg = mainImageArgs[i];
            if (arg.Contains("out ")) {
              if (arg.Contains("float4")) {
                retVarName = arg.Substring(arg.IndexOf("float4 ") + 7);
              }
              arg = arg.Replace("out ", "") + " = 0";
            } else {
              arg = arg.Replace("in ", "") + " = uv";
            }
            sbHeader.AppendLine(arg + ";");
          }
        }

        sbHeader.AppendLine("// CONV: Center on screen, then try some aspect correction");
        sbHeader.AppendLine("uv = (uv*2) - 1;");
        sbHeader.AppendLine("uv *= aspect.xy;");
        sbHeader.AppendLine("// CONV: Adjust this to flip the output (±uv.x, ±uv.y)");
        sbHeader.AppendLine("uv = float2(uv.x, -uv.y);");

        int indexMainImageMethodStartingBracket = inpMain.IndexOf("{");
        inpMain = sbHeader.ToString() + inpMain.Substring(indexMainImageMethodStartingBracket + 1);

        inp = inpHeader + inpMain;

        // global processing of all lines
        string[] lines = inp.Replace("\r\n", "\n").Replace('\r', '\n').Split(new[] { '\n' }, StringSplitOptions.None);
        foreach (string line in lines) {
          string currentLine = line;
          if (line.Contains("float2 uv =")) {
            currentLine = indent + "// " + line;
          } else if (line.Contains("iMouse")) {
            SetConvertorError("iMouse unsupported", sb);
            currentLine = indent + "// " + line;
          } else if (line.Contains("iDate")) {
            SetConvertorError("iDate unsupported", sb);
            currentLine = indent + "// " + line;
          } else if (line.Contains("iChannel")) {
            SetConvertorError("iChannel (textures) unsupported", sb);
            currentLine = indent + "// " + line;
          } else if (line.Contains("break;")) {
            SetConvertorError("break unsupported, see Milkwave manual", sb);
          }

          currentLine = FixMatrixMultiplication(currentLine);
          currentLine = FixFloatNumberOfArguments(currentLine, inp);

          sb.AppendLine(currentLine);
        }
        result = sb.ToString();
        AddReturnValue(ref result, retVarName);

      } catch (Exception e) {
        Debug.Assert(false);
      }

      return result;
    }

    private string StripCommentsAndBlankLines(string inp) {
      StringBuilder sb = new StringBuilder();
      string[] lines = inp.Replace("\r\n", "\n").Replace('\r', '\n').Split(new[] { '\n' }, StringSplitOptions.None);
      bool inComment = false;
      string prevLine = "";
      foreach (string line in lines) {
        string currentLine = line.Trim();
        if (currentLine.StartsWith("//")) {
          // skip comments before the code starts
          continue;
        } else if (currentLine.StartsWith("/*")) {
          inComment = !currentLine.EndsWith("*/");
          continue;
        } else if (currentLine.EndsWith("*/")) {
          inComment = false;
          continue;
        } else if (inComment) {
          continue;
        } else if (currentLine.Length > 0 || prevLine.Length > 0) {
          // allow a single blank line, but not multiple blank lines
          sb.AppendLine(currentLine);
        }
        prevLine = currentLine;
      }
      return sb.ToString();
    }

    public string FixFloatNumberOfArguments(string inputLine, string fullContext, int startIndex = 0) {
      string result = inputLine;
      for (int numArgs = 2; numArgs <= 4; numArgs++) {
        int index = result.IndexOf("float" + numArgs + "(", startIndex);
        if (index > -1) {
          string restOfLine = result.Substring(index + 7);
          // find closing bracket
          int indexcClosingBracket = FindClosingBracketIndex(restOfLine, '(', ')', 1);
          if (indexcClosingBracket > 0) {
            string argsLine = restOfLine.Substring(0, indexcClosingBracket);
            string[] args = argsLine.Split(",");
            if (args.Length == 1) {
              // argument could be a vector
              if (float.TryParse(argsLine, out float dummy)
                // number argument should be multiplied
                || (args[0].Contains("(") && args[0].Contains(")"))
                // function call argument should be multiplied
                || (fullContext.Contains("float " + args[0] + ",") || fullContext.Contains("float " + args[0] + ";"))
                // float variables should be multiplied
                ) {
                string newArgsLine = argsLine;
                // we expected numArgs arguments
                for (int i = 1; i < numArgs; i++) {
                  newArgsLine += ", " + argsLine;
                }
                result = result.Substring(0, index + 7)
                  + newArgsLine
                  + result.Substring(index + 7 + indexcClosingBracket);
                //result = FixFloatNumberOfArguments(result, fullContext, index + 7 + newArgsLine.Length);
              }
            }
          }
        }
      }
      return result;
    }

    public string FixMatrixMultiplication(string inputLine) {
      string result = inputLine;
      inputLine = inputLine.Replace("*= mat", "*=mat").Replace("* mat", "*mat").Replace(" *mat", "*mat");
      string token = "*=mat";
      int index = inputLine.IndexOf(token);
      if (index > -1) {
        // something like "uv *= mat2(cos(angle), -sin(angle), sin(angle), cos(angle));" (see ShaderTests.cs)
        string matSizeChar = inputLine.Substring(index + token.Length, 1);
        if (int.TryParse(matSizeChar, out int matSize)) {
          string fac1 = inputLine.Substring(0, index).Trim();
          string indent = inputLine.Substring(0, inputLine.IndexOf(fac1));
          int closingBracketIndex = FindClosingBracketIndex(inputLine.Substring(index + token.Length + 2), '(', ')', 1);
          string args = inputLine.Substring(index + token.Length + 2, closingBracketIndex).Trim();
          result = indent + fac1 + " = mul(" + fac1 + ", transpose(float" + matSize + "x" + matSize + "(" + args + ")));";
        }
      } else {
        token = "*mat";
        index = inputLine.IndexOf(token);
        if (index > -1) {
          // something like "return p*mat2(c,s,-s,c);" (see ShaderTests.cs)
          string matSizeChar = inputLine.Substring(index + token.Length, 1);
          if (int.TryParse(matSizeChar, out int matSize)) {
            string fac1 = inputLine.Substring(0, index).Trim();
            int blankIndex = fac1.LastIndexOf(' ');
            if (blankIndex > -1) {
              fac1 = fac1.Substring(blankIndex + 1);
            }
            int closingBracketIndex = FindClosingBracketIndex(inputLine.Substring(index + token.Length + 2), '(', ')', 1);
            string args = inputLine.Substring(index + token.Length + 2, closingBracketIndex).Trim();
            string left = inputLine.Substring(0, index - fac1.Length);
            result = left + " mul(" + fac1 + ", transpose(float" + matSize + "x" + matSize + "(" + args + ")));";
            result = result.Replace("  ", " ");
          }
        }
      }

      // try to replace any remaining clear uses of mat2, mat3, mat4
      result = result.Replace("mat2(", "float2x2(").Replace("mat3(", "float3x3(").Replace("mat4(", "float4x4(");
      result = result.Replace("mat2 ", "float2x2 ").Replace("mat3 ", "float3x3 ").Replace("mat4 ", "float4x4 ");

      return result;
    }

    private int FindClosingBracketIndex(string input, char openingBracket, char closingBracket, int level) {
      int bracketCount = level;
      for (int i = 0; i < input.Length; i++) {
        if (input[i] == openingBracket) {
          bracketCount++;
        } else if (input[i] == closingBracket) {
          bracketCount--;
          if (bracketCount == 0) {
            return i;
          }
        }
      }
      return -1; // No matching closing bracket found
    }

    private void SetConvertorError(string msg, StringBuilder sb) {
      sb.AppendLine("// CONV: " + msg);
      if (!ConversionErrors.ToString().Contains(msg + Environment.NewLine)) {
        ConversionErrors.AppendLine(msg);
      }
    }

    private string AddHelperFunctionsMod(string inpHeader) {
      StringBuilder sb = new StringBuilder();
      sb.AppendLine("float mod_conv(float x, float y) { return x - y * floor(x / y); }");
      sb.AppendLine("float2 mod_conv(float2 x, float2 y) { return x - y * floor(x / y); }");
      sb.AppendLine("float3 mod_conv(float3 x, float3 y) { return x - y * floor(x / y); }");
      sb.AppendLine("float4 mod_conv(float4 x, float4 y) { return x - y * floor(x / y); }");
      return sb.ToString() + Environment.NewLine + inpHeader;
    }

    private string AddHelperFunctionsLessThan(string inpHeader) {
      StringBuilder sb = new StringBuilder();
      sb.AppendLine("float4 lessThan(float4 a, float4 b) { return float4(a.x < b.x ? 1.0 : 0.0, a.y < b.y ? 1.0 : 0.0, a.z < b.z ? 1.0 : 0.0, a.w < b.w ? 1.0 : 0.0); }");
      return sb.ToString() + Environment.NewLine + inpHeader;
    }

    private string ReplaceVarName(string oldName, string newName, string inp) {
      string res = inp.Replace(" " + oldName + " ", " " + newName + " ");
      res = res.Replace("" + oldName + ".", "" + newName + ".");
      res = res.Replace("(" + oldName + ".", "(" + newName + ".");
      res = res.Replace("(" + oldName + " ", "(" + newName + " ");
      res = res.Replace(" " + oldName + ")", " " + newName + ")");
      res = res.Replace("(" + oldName + ")", "(" + newName + ")");
      res = res.Replace(oldName + "=", newName + "=");
      res = res.Replace(oldName + " =", newName + " =");
      res = res.Replace(oldName + "+", newName + "+");
      res = res.Replace(oldName + " +", newName + " +");
      res = res.Replace("float2 " + oldName + ",", "float2 " + newName + ", ");
      res = res.Replace("float2 " + oldName + ";", "float2 " + newName + "; ");
      res = res.Replace("float2 " + oldName + " ", "float2 " + newName + " ");
      return res;
    }

    void AddReturnValue(ref string inp, string varName) {
      try {
        int ind = inp.IndexOf("shader_body");
        if (ind >= 0) {
          int indShaderBodyClosingBracket = FindClosingBracketIndex(inp.Substring(ind), '{', '}', 0);
          if (indShaderBodyClosingBracket >= 0) {
            indShaderBodyClosingBracket += ind;
            string p1 = inp.Substring(0, indShaderBodyClosingBracket);
            string p2 = inp.Substring(indShaderBodyClosingBracket);
            inp = p1 + "ret = " + varName + ";" + Environment.NewLine + p2;
          }
        }
      } catch {}
    }

  } // end class
} // end namespace
