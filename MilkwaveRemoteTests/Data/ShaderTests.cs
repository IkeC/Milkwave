using System.Diagnostics;

namespace MilkwaveRemote.Data.Tests {
  [TestClass()]
  public class ShaderTests {

    private Shader shader = new Shader();

    [TestMethod()]
    public void FixMatrixMultiplicationTest() {
      string arg = " uv *= mat2(cos(angle), -sin(angle), sin(angle), cos(angle));";
      string expected = " uv = mul(uv, transpose(float2x2(cos(angle), -sin(angle), sin(angle), cos(angle))));";
      string res = shader.FixMatrixMultiplication(arg);
      Debug.Print(expected + Environment.NewLine + res);
      Assert.IsTrue(res.Equals(expected));

      arg = "return p*mat2(c,s,-s,c);";
      expected = "return mul(p, transpose(float2x2(c,s,-s,c)));";
      res = shader.FixMatrixMultiplication(arg);
      Debug.Print(expected + Environment.NewLine + res);
      Assert.IsTrue(res.Equals(expected));

      arg = "return longvarname * mat2(c,s,-s,c);";
      expected = "return mul(longvarname, transpose(float2x2(c,s,-s,c)));";
      res = shader.FixMatrixMultiplication(arg);
      Debug.Print(expected + Environment.NewLine + res);
      Assert.IsTrue(res.Equals(expected));
    }

  }
}