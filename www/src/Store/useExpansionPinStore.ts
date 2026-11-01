import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues, PinDirectionValues } from '../Data/Pins';

export type ExpansionPin = {
	option: PinActionValues;
	direction: PinDirectionValues;
	customButtonMask?: number;
	customDpadMask?: number;
};

type State = {
	pins: {
		[key: string]: [
			{
				[key: string]: ExpansionPin;
			},
		];
	};
	loadingPins: boolean;
};

type Actions = {
	fetchPins: () => void;
	setPinAction: (
		expansion: string,
		index: number,
		pin: string,
		action: PinActionValues,
	) => void;
	setPinDirection: (
		expansion: string,
		index: number,
		pin: string,
		dir: PinDirectionValues,
	) => void;
	setPin: (
		expansion: string,
		index: number,
		pin: string,
		data: Partial<ExpansionPin>,
	) => void;
	savePins: () => Promise<object>;
};

const INITIAL_STATE: State = {
	pins: {
		pcf8575: [
			{
				pin00: { option: -10, direction: 0 },
				pin01: { option: -10, direction: 0 },
				pin02: { option: -10, direction: 0 },
				pin03: { option: -10, direction: 0 },
				pin04: { option: -10, direction: 0 },
				pin05: { option: -10, direction: 0 },
				pin06: { option: -10, direction: 0 },
				pin07: { option: -10, direction: 0 },
				pin08: { option: -10, direction: 0 },
				pin09: { option: -10, direction: 0 },
				pin10: { option: -10, direction: 0 },
				pin11: { option: -10, direction: 0 },
				pin12: { option: -10, direction: 0 },
				pin13: { option: -10, direction: 0 },
				pin14: { option: -10, direction: 0 },
				pin15: { option: -10, direction: 0 },
			},
		],
		shiftRegisterInput: [
			{
				pin00: { option: -10, direction: 0 },
				pin01: { option: -10, direction: 0 },
				pin02: { option: -10, direction: 0 },
				pin03: { option: -10, direction: 0 },
				pin04: { option: -10, direction: 0 },
				pin05: { option: -10, direction: 0 },
				pin06: { option: -10, direction: 0 },
				pin07: { option: -10, direction: 0 },
				pin08: { option: -10, direction: 0 },
				pin09: { option: -10, direction: 0 },
				pin10: { option: -10, direction: 0 },
				pin11: { option: -10, direction: 0 },
				pin12: { option: -10, direction: 0 },
				pin13: { option: -10, direction: 0 },
				pin14: { option: -10, direction: 0 },
				pin15: { option: -10, direction: 0 },
				pin16: { option: -10, direction: 0 },
				pin17: { option: -10, direction: 0 },
				pin18: { option: -10, direction: 0 },
				pin19: { option: -10, direction: 0 },
				pin20: { option: -10, direction: 0 },
				pin21: { option: -10, direction: 0 },
				pin22: { option: -10, direction: 0 },
				pin23: { option: -10, direction: 0 },
				pin24: { option: -10, direction: 0 },
				pin25: { option: -10, direction: 0 },
				pin26: { option: -10, direction: 0 },
				pin27: { option: -10, direction: 0 },
				pin28: { option: -10, direction: 0 },
				pin29: { option: -10, direction: 0 },
				pin30: { option: -10, direction: 0 },
				pin31: { option: -10, direction: 0 },
			},
		],
	},
	loadingPins: false,
};

const useExpansionPinStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchPins: async () => {
		set({ loadingPins: true });
		const pins = await WebApi.getExpansionPins();
		set((state) => ({
			...state,
			...pins,
			loadingPins: false,
		}));
	},
	setPinAction: (expansion, index, pin, action) => {
		set((state) => {
			const newPins = { ...state.pins };

			if (
				newPins[expansion] &&
				newPins[expansion][index] &&
				newPins[expansion][index][pin]
			) {
				newPins[expansion][index] = {
					...newPins[expansion][index],
					[pin]: {
						...newPins[expansion][index][pin],
						option: action,
					},
				};
			}

			return {
				...state,
				pins: newPins,
			};
		});
	},
	setPinDirection: (expansion, index, pin, dir) => {
		set((state) => {
			const newPins = { ...state.pins };

			if (
				newPins[expansion] &&
				newPins[expansion][index] &&
				newPins[expansion][index][pin]
			) {
				newPins[expansion][index] = {
					...newPins[expansion][index],
					[pin]: {
						...newPins[expansion][index][pin],
						direction: dir,
					},
				};
			}

			return {
				...state,
				pins: newPins,
			};
		});
	},
	setPin: (expansion, index, pin, data) => {
		set((state) => {
			const newPins = { ...state.pins };

			if (
				newPins[expansion] &&
				newPins[expansion][index] &&
				newPins[expansion][index][pin]
			) {
				newPins[expansion][index] = {
					...newPins[expansion][index],
					[pin]: {
						...newPins[expansion][index][pin],
						...data,
					},
				};
			}

			return {
				...state,
				pins: newPins,
			};
		});
	},
	savePins: async () => WebApi.setExpansionPins(get()),
}));

export default useExpansionPinStore;
