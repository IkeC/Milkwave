using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using MilkwaveRemote.Helper;

namespace MilkwaveRemote {
  public partial class MilkwaveInfoForm : Form {
    private readonly bool darkModeEnabled;
    private readonly List<LinkSpan> customLinks = new();
    private Font? linkFont;

    [DllImport("user32.dll")]
    static extern bool HideCaret(IntPtr hWnd);

    public MilkwaveInfoForm(bool darkMode) {
      InitializeComponent();
      darkModeEnabled = darkMode;
      dm = new DarkModeCS(this) {
        ColorMode = darkMode ? DarkModeCS.DisplayMode.DarkMode : DarkModeCS.DisplayMode.ClearMode,
      };
    }

    public void ShowDialog(string title, string text, IEnumerable<(string Caption, string Url)>? links = null) {
      Text = title;
      SetTextContent(text, links);
      ShowDialog();
    }

    public void ShowDialog(string title, string text, float fontSize, int width, int height, IEnumerable<(string Caption, string Url)>? links = null) {
      Text = title;
      textBox.Font = new Font(textBox.Font.FontFamily, fontSize);
      SetTextContent(text, links);
      this.Width = width;
      this.Height = height;
      ShowDialog();
    }

    private void MilkwaveInfoForm_Shown(object sender, EventArgs e) {
      btnClose.Focus();
    }

    private void btnClose_Click(object sender, EventArgs e) {
      Close();
    }

    private void textBox_LinkClicked(object sender, LinkClickedEventArgs e) {
      OpenUrl(e.LinkText);
    }

    private void textBox_MouseMove(object sender, MouseEventArgs e) {
      if (TryGetLinkAtPoint(e.Location, out _)) {
        textBox.Cursor = Cursors.Hand;
      }
      else {
        textBox.Cursor = Cursors.Default;
      }
    }

    private void textBox_MouseLeave(object sender, EventArgs e) {
      textBox.Cursor = Cursors.Default;
    }

    private void textBox_MouseClick(object sender, MouseEventArgs e) {
      if (TryGetLinkAtPoint(e.Location, out var link)) {
        OpenUrl(link.Url);
      }
    }

    private void OpenUrl(string? url) {
      if (string.IsNullOrWhiteSpace(url)) {
        return;
      }

      Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
    }

    private void SetTextContent(string text, IEnumerable<(string Caption, string Url)>? links) {
      bool hasCustomLinks = links != null;
      textBox.DetectUrls = !hasCustomLinks;

      if (!string.IsNullOrWhiteSpace(text) && text.TrimStart().StartsWith("{\\rtf", StringComparison.OrdinalIgnoreCase)) {
        textBox.Rtf = text;
      }
      else {
        textBox.Text = text;
      }

      customLinks.Clear();

      if (links != null) {
        Color linkColor = darkModeEnabled ? Color.Gold : SystemColors.HotTrack;
        foreach (var link in links) {
          HighlightLink(link.Caption, link.Url, linkColor);
        }
        textBox.Select(0, 0);
      }
    }

    private void HighlightLink(string caption, string url, Color color) {
      if (string.IsNullOrWhiteSpace(caption) || string.IsNullOrWhiteSpace(url)) {
        return;
      }

      string fullText = textBox.Text;
      int startIndex = 0;
      while (startIndex < fullText.Length) {
        int matchIndex = fullText.IndexOf(caption, startIndex, StringComparison.Ordinal);
        if (matchIndex < 0) {
          break;
        }

        textBox.Select(matchIndex, caption.Length);
        EnsureLinkFont();
        textBox.SelectionColor = color;
        textBox.SelectionFont = linkFont!;
        customLinks.Add(new LinkSpan(matchIndex, caption.Length, url));
        startIndex = matchIndex + caption.Length;
      }
    }

    private void EnsureLinkFont() {
      if (linkFont == null) {
        linkFont = new Font(textBox.Font, FontStyle.Underline);
      }
    }

    private bool TryGetLinkAtPoint(Point location, out LinkSpan link) {
      link = default;
      if (customLinks.Count == 0) {
        return false;
      }

      int charIndex = textBox.GetCharIndexFromPosition(location);
      if (charIndex < 0) {
        return false;
      }

      foreach (var span in customLinks) {
        if (charIndex >= span.Start && charIndex < span.Start + span.Length) {
          link = span;
          return true;
        }
      }

      return false;
    }

    private readonly record struct LinkSpan(int Start, int Length, string Url);
  }

}
