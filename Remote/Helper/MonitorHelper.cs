using System.Diagnostics;

namespace MilkwaveRemote.Helper {
  internal static class MonitorHelper {
    // GPU counters
    private static Lazy<List<PerformanceCounter>> GpuCounters
        = new Lazy<List<PerformanceCounter>>(InitGpuCounters, true);

    // CPU counter
    private static Lazy<PerformanceCounter?> CpuCounter
        = new Lazy<PerformanceCounter?>(InitCpuCounter, isThreadSafe: true);

    // Initialize and warm up GPU counters
    private static List<PerformanceCounter> InitGpuCounters() {
      var counters = new List<PerformanceCounter>();
      try {
        var cat = new PerformanceCounterCategory("GPU Engine");
        counters = cat
            .GetInstanceNames()
            .SelectMany(inst => cat.GetCounters(inst))
            .Where(c => c.CounterName == "Utilization Percentage")
            .ToList();

        // First NextValue often returns 0
        counters.ForEach(c => _ = c.NextValue());
      } catch (Exception ex) {
        Debug.WriteLine($"Failed to initialize GPU counters: {ex.Message}");
      }
      return counters;
    }

    // Initialize and warm up CPU counter
    private static PerformanceCounter? InitCpuCounter() {
      try {
        var cpu = new PerformanceCounter("Processor", "% Processor Time", "_Total", readOnly: true);
        // First call returns 0, so prime it
        _ = cpu.NextValue();
        return cpu;
      } catch (Exception ex) {
        Debug.WriteLine($"Failed to initialize CPU counter: {ex.Message}");
        return null;
      }
    }

    // Returns the sum of NextValue() over all GPU engines (percent)
    public static float GetGPUUsage() {
      float res = -1f;
      try {
        var counters = GpuCounters.Value;
        if (counters != null && counters.Any()) {
          // Sum across all engines
          res = counters.Sum(c => c.NextValue());
        }
      } catch (Exception e) {
        GpuCounters = new Lazy<List<PerformanceCounter>>(InitGpuCounters, true);
      }
      return res;
    }

    // Returns current total CPU utilization (percent)
    public static float GetCPUUsage() {
      float res = -1f;
      try {
        var counter = CpuCounter.Value;
        if (counter != null) {
          res = counter.NextValue();
        }
      } catch (Exception e) {
        CpuCounter = new Lazy<PerformanceCounter?>(InitCpuCounter, isThreadSafe: true);
      }
      return res;
    }
  }
}
