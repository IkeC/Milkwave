using System.Text;

namespace MilkwaveRemote.Helper {
  internal sealed class RtfBuilder {
    private const string Footer = "}";
    private const string BaseHeader = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Segoe UI;}}\\f0\\fs20 ";
    private const string DarkHeader = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Segoe UI;}}{\\colortbl ;\\red255\\green215\\blue0;}\\f0\\fs20 ";

    private readonly StringBuilder body = new();
    private readonly string header;
    private readonly string linkColorPrefix;
    private readonly string linkColorReset;

    private RtfBuilder(bool darkMode) {
      if (darkMode) {
        header = DarkHeader;
        linkColorPrefix = "\\cf1 ";
        linkColorReset = "\\cf0 ";
      }
      else {
        header = BaseHeader;
        linkColorPrefix = string.Empty;
        linkColorReset = string.Empty;
      }
    }

    public static RtfBuilder Create(bool darkMode = false) => new RtfBuilder(darkMode);

    public RtfBuilder AppendText(string text) {
      body.Append(EscapeText(text));
      return this;
    }

    public RtfBuilder AppendLink(string label, string url) {
      _ = url; // URL handled externally when rendering
      body.Append(EscapeText("• "));
      body.Append(EscapeText(label));
      return this;
    }

    public RtfBuilder AppendLine(int count = 1) {
      for (int i = 0; i < count; i++) {
        body.Append("\\line ");
      }
      return this;
    }

    public RtfBuilder AppendParagraphBreak() {
      return AppendLine(2);
    }

    public string Build() {
      return header + body.ToString() + Footer;
    }

    private static string EscapeText(string text) {
      var sb = new StringBuilder(text.Length);
      foreach (char ch in text) {
        switch (ch) {
          case '\\':
            sb.Append("\\\\");
            break;
          case '{':
            sb.Append("\\{");
            break;
          case '}':
            sb.Append("\\}");
            break;
          case '\r':
            break;
          case '\n':
            sb.Append("\\line ");
            break;
          default:
            if (ch <= 0x7F) {
              sb.Append(ch);
            }
            else {
              sb.Append("\\u").Append((int)ch).Append('?');
            }
            break;
        }
      }
      return sb.ToString();
    }

    private static string EscapeAttribute(string text) {
      var sb = new StringBuilder(text.Length);
      foreach (char ch in text) {
        switch (ch) {
          case '\\':
            sb.Append("\\\\");
            break;
          case '"':
            sb.Append("\\\"");
            break;
          default:
            sb.Append(ch);
            break;
        }
      }
      return sb.ToString();
    }
  }
}
