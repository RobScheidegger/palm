import create from "zustand";

export enum Hand {
  Left = "left",
  Right = "right",
}

export interface RobotState {
  position: {
    x: number;
    y: number;
    z: number;
  };
  update: (newState: Partial<RobotState>) => void;
}

export const useRobot = create<RobotState>((set, get) => ({
  position: { x: 0, y: 0, z: 0 },
  update: (newState: Partial<RobotState>) => set({ ...newState }),
}));
