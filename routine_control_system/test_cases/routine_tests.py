import unittest
from routine_control_system.routine_manager import RoutineManager


class RoutineManagerTests(unittest.TestCase):

    def test_complete_routine(self):
        rm = RoutineManager(study_duration=1, break_duration=1, total_cycles=2)
        rm.start()
        self.assertEqual(rm.current_cycle, 2)
        self.assertEqual(rm.state, "FINISHED")

    def test_pause_and_resume(self):
        rm = RoutineManager(study_duration=3, break_duration=1, total_cycles=1)
        rm.start()
        rm.pause()
        self.assertTrue(rm.is_paused)
        rm.resume()
        self.assertFalse(rm.is_paused)

    def test_seated_state_change(self):
        rm = RoutineManager(study_duration=2, break_duration=1, total_cycles=1)
        rm.set_seated(False)
        self.assertFalse(rm.is_seated)
        self.assertEqual(rm.state, "PAUSED")
        rm.set_seated(True)
        self.assertTrue(rm.is_seated)
        self.assertIn(rm.state, ["STUDY", "BREAK"])

if __name__ == "__main__":
    unittest.main()
