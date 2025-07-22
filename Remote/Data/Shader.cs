using System.Diagnostics;
using System.Text;

namespace MilkwaveRemote.Data {
  public class Shader {
    private static string indent = "";

    public static string ConvertGLSLtoHLSL(string inp) {
      StringBuilder sb = new StringBuilder();

      try {
        inp = inp.Replace("vec2", "float2").Replace("vec3", "float3").Replace("vec4", "float4");
        inp = inp.Replace("fract (", "fract(").Replace("mod (", "mod(").Replace("mix (", "mix (");
        inp = inp.Replace("fract(", "frac(").Replace("mod(", "mod_conv(").Replace("mix(", "lerp(");
        inp = inp.Replace("iTime", "time").Replace("iResolution", "texsize");
        inp = inp.Replace("void mainImage(", "mainImage(");

        int indexMainImage = inp.IndexOf("mainImage(");

        string inpHeader = "";
        string inpMain = "";
        string inpFooter = "";

        if (indexMainImage == -1) {
          // no mainImage function, we'll just wrap everything into a shader_body
          inpMain = inp + Environment.NewLine + "}"; // opening bracket is supplied below
        } else {
          int indexMainImageMethodClosingBracket = FindClosingBracketIndex(inp.Substring(indexMainImage), '{', '}', 0);

          inpHeader = inp.Substring(0, indexMainImage);
          inpMain = inp.Substring(indexMainImage, indexMainImageMethodClosingBracket + 1);
          inpFooter = "";

          int footerIndex = indexMainImage + indexMainImageMethodClosingBracket + 1;
          if (inp.Length > footerIndex) {
            inpFooter = inp.Substring(footerIndex);
          }
          inpHeader += inpFooter;
        }

        inpHeader = inpHeader.Replace(" uv ", " uv_conv ");
        inpHeader = inpHeader.Replace(" uv.", " uv_conv.");
        inpHeader = inpHeader.Replace("(uv", "(uv_conv");
        inpHeader = inpHeader.Replace("uv)", "uv_conv)");
        inpHeader = inpHeader.Replace("float2 uv", "float2 uv_conv");

        if (inp.Contains("mod_conv(")) {
          inpHeader = AddHelperFunctionsMod(inpHeader);
        }
        if (inp.Contains("lessthan", StringComparison.InvariantCultureIgnoreCase)) {
          inpHeader = AddHelperFunctionsLessThan(inpHeader);
        }

        StringBuilder sbHeader = new StringBuilder();
        sbHeader.AppendLine("shader_body {");

        if (indexMainImage > -1) {
          // if we have a mainImage function, convert its arguments

          int indexMainImageArgsOpeningBracket = inpMain.IndexOf("(") + 1;
          int indexMainImageArgsClosingBracket = FindClosingBracketIndex(inpMain.Substring(indexMainImageArgsOpeningBracket), '(', ')', 1);

          string mainImageArgsString = inpMain.Substring(indexMainImageArgsOpeningBracket, indexMainImageArgsClosingBracket);
          string[] mainImageArgs = mainImageArgsString.Split(",");
          for (int i = 0; i < mainImageArgs.Length; i++) {
            string arg = mainImageArgs[i].Replace("in ", "").Replace("out ", "").Trim();
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
          if (line.Contains("fragColor =")) {
            currentLine = line.Replace("fragColor =", "ret =");
          } else if (line.Contains("float2 uv =")) {
            currentLine = indent + "// " + line;
          } else if (line.Contains("*= mat")) {
            // TODO
            SetConvertorError("Matrix multiplication unsupported", sb);
            SetConvertorError("Example: uv = mul(uv,transpose(float2x2(arg1, arg2, arg3, arg4)));", sb);
            currentLine = indent + "// " + line;
          } else if (line.Contains("iMouse")) {
            SetConvertorError("iMouse unsupported", sb);
            currentLine = indent + "// " + line;
          } else if (line.Contains("iChannel")) {
            SetConvertorError("iChannel (textures) unsupported", sb);
            currentLine = indent + "// " + line;
          }

          currentLine = FixFloatNumberOfArguments(currentLine, inp);
          sb.AppendLine(currentLine);
        }
      } catch (Exception e) {
        Debug.Assert(false);
      }
      return sb.ToString();
    }

    private static string FixFloatNumberOfArguments(string inputLine, string fullContext, int startIndex = 0) {
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

    private static int FindClosingBracketIndex(string input, char openingBracket, char closingBracket, int level) {
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

    private static void SetConvertorError(string msg, StringBuilder sb) {
      sb.AppendLine("// CONV: " + msg);
    }

    private static string AddHelperFunctionsMod(string inpHeader) {
      StringBuilder sb = new StringBuilder();
      sb.AppendLine("float mod_conv(float x, float y) { return x - y * floor(x / y); }");
      sb.AppendLine("float2 mod_conv(float2 x, float2 y) { return x - y * floor(x / y); }");
      sb.AppendLine("float3 mod_conv(float3 x, float3 y) { return x - y * floor(x / y); }");
      sb.AppendLine("float4 mod_conv(float4 x, float4 y) { return x - y * floor(x / y); }");
      return sb.ToString() + Environment.NewLine + inpHeader;
    }

    private static string AddHelperFunctionsLessThan(string inpHeader) {
      StringBuilder sb = new StringBuilder();
      sb.AppendLine("float4 lessThan(float4 a, float4 b) { return float4(a.x < b.x ? 1.0 : 0.0, a.y < b.y ? 1.0 : 0.0, a.z < b.z ? 1.0 : 0.0, a.w < b.w ? 1.0 : 0.0); }");
      return sb.ToString() + Environment.NewLine + inpHeader;
    }
  } // end class
} // end namespace
