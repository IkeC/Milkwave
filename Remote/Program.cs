namespace MilkwaveRemote {
  internal static class Program {
    /// <summary>
    ///  The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main() {
      // To customize application configuration such as set high DPI settings or default font,
      // see https://aka.ms/applicationconfiguration.
      ApplicationConfiguration.Initialize();

      Application.ThreadException += new ThreadExceptionEventHandler(Application_ThreadException);
      Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
      AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
      Application.EnableVisualStyles();
      Application.Run(new MilkwaveRemoteForm());
    }

    static void Application_ThreadException(object sender, ThreadExceptionEventArgs e) {
      try {
        SaveErrorToFile(e.Exception, "Unhandled Thread Exception");
      } catch (Exception ex) {
        // Handle any errors that occur while logging
        MessageBox.Show($"Failed to write error log: {ex.Message}" +
          Environment.NewLine + Environment.NewLine + e.Exception?.StackTrace, "Error");
      } finally {
        Environment.Exit(1);
      }
    }

    static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e) {
      try {
        if (e.ExceptionObject is Exception ex) { // Use pattern matching to fix IDE0019
          SaveErrorToFile(ex, "Unhandled Exception");
        } else {
          MessageBox.Show("An unknown error occurred.", "Unhandled Exception");
        }
      } catch (Exception ex) {
        // Handle any errors that occur while logging  
        MessageBox.Show($"Failed to write error log: {ex.Message}" +
          Environment.NewLine + Environment.NewLine + ((Exception)e.ExceptionObject).StackTrace, "Error");
      } finally {
        Environment.Exit(1);
      }
    }

    public static void SaveErrorToFile(Exception e, string type) {
      LogToFile($"{type}: {e}");
      // Notify the user
      string exeDirectory = AppDomain.CurrentDomain.BaseDirectory;
      string timestamp = DateTime.Now.ToString("yyyy-MM-dd");
      string logFilePath = Path.Combine(exeDirectory, "log", $"{timestamp}.remote.log");
      MessageBox.Show($"An error occurred. Details have been saved to:\n{logFilePath}", type);
    }

    public static void LogToFile(string message) {
      try {
        string exeDirectory = AppDomain.CurrentDomain.BaseDirectory;
        string timestamp = DateTime.Now.ToString("yyyy-MM-dd");
        string logDir = Path.Combine(exeDirectory, "log");
        if (!Directory.Exists(logDir)) {
          Directory.CreateDirectory(logDir);
        }
        string logFilePath = Path.Combine(logDir, $"{timestamp}.remote.log");
        string formattedMessage = $"{DateTime.Now:HH:mm:ss.fff} - {message}{Environment.NewLine}";
        File.AppendAllText(logFilePath, formattedMessage);
      } catch {
        // Ignore logging errors to prevent crashes
      }
    }
  }
}