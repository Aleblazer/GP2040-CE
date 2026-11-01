import { useCallback, useContext, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, Button, FormCheck, Row } from 'react-bootstrap';
import { MultiValue, SingleValue } from 'react-select';
import * as yup from 'yup';
import omit from 'lodash/omit';

import { AppContext } from '../Contexts/AppContext';
import useExpansionPinStore, {
	ExpansionPin,
} from '../Store/useExpansionPinStore';
import useBoardDefinition from '../Store/useBoardDefinitionStore';

import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';

import { BUTTON_MASKS, DPAD_MASKS, getButtonLabels } from '../Data/Buttons';
import { BUTTON_ACTIONS, PinActionKeys, PinActionValues } from '../Data/Pins';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';
import './ShiftRegisterInput.scss';

const EXPANSION_TYPE = 'shiftRegisterInput';
const MAX_CHIPS = 4;
const INPUT_LETTERS = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'];

// Actions the firmware honors for shift register inputs: gamepad buttons, plus the
// actions other features look up through Gamepad::isActionPressed
const SUPPORTED_ACTION_KEYS: PinActionKeys[] = [
	'BUTTON_PRESS_UP',
	'BUTTON_PRESS_DOWN',
	'BUTTON_PRESS_LEFT',
	'BUTTON_PRESS_RIGHT',
	'BUTTON_PRESS_B1',
	'BUTTON_PRESS_B2',
	'BUTTON_PRESS_B3',
	'BUTTON_PRESS_B4',
	'BUTTON_PRESS_L1',
	'BUTTON_PRESS_R1',
	'BUTTON_PRESS_L2',
	'BUTTON_PRESS_R2',
	'BUTTON_PRESS_S1',
	'BUTTON_PRESS_S2',
	'BUTTON_PRESS_A1',
	'BUTTON_PRESS_A2',
	'BUTTON_PRESS_L3',
	'BUTTON_PRESS_R3',
	'BUTTON_PRESS_A3',
	'BUTTON_PRESS_A4',
	'BUTTON_PRESS_FN',
	'BUTTON_PRESS_E1',
	'BUTTON_PRESS_E2',
	'BUTTON_PRESS_E3',
	'BUTTON_PRESS_E4',
	'BUTTON_PRESS_E5',
	'BUTTON_PRESS_E6',
	'BUTTON_PRESS_E7',
	'BUTTON_PRESS_E8',
	'BUTTON_PRESS_E9',
	'BUTTON_PRESS_E10',
	'BUTTON_PRESS_E11',
	'BUTTON_PRESS_E12',
	'DIGITAL_DIRECTION_UP',
	'DIGITAL_DIRECTION_DOWN',
	'DIGITAL_DIRECTION_LEFT',
	'DIGITAL_DIRECTION_RIGHT',
	'ANALOG_DIRECTION_LS_X_NEG',
	'ANALOG_DIRECTION_LS_X_POS',
	'ANALOG_DIRECTION_LS_Y_NEG',
	'ANALOG_DIRECTION_LS_Y_POS',
	'ANALOG_DIRECTION_RS_X_NEG',
	'ANALOG_DIRECTION_RS_X_POS',
	'ANALOG_DIRECTION_RS_Y_NEG',
	'ANALOG_DIRECTION_RS_Y_POS',
	'BUTTON_PRESS_TURBO',
	'BUTTON_PRESS_MACRO',
	'BUTTON_PRESS_MACRO_1',
	'BUTTON_PRESS_MACRO_2',
	'BUTTON_PRESS_MACRO_3',
	'BUTTON_PRESS_MACRO_4',
	'BUTTON_PRESS_MACRO_5',
	'BUTTON_PRESS_MACRO_6',
	'BUTTON_PRESS_DDI_UP',
	'BUTTON_PRESS_DDI_DOWN',
	'BUTTON_PRESS_DDI_LEFT',
	'BUTTON_PRESS_DDI_RIGHT',
	'SUSTAIN_DP_MODE_DP',
	'SUSTAIN_DP_MODE_LS',
	'SUSTAIN_DP_MODE_RS',
	'SUSTAIN_SOCD_MODE_UP_PRIO',
	'SUSTAIN_SOCD_MODE_NEUTRAL',
	'SUSTAIN_SOCD_MODE_SECOND_WIN',
	'SUSTAIN_SOCD_MODE_FIRST_WIN',
	'SUSTAIN_SOCD_MODE_BYPASS',
	'SUSTAIN_4_8_WAY_MODE',
	'SUSTAIN_FOCUS_MODE',
	'BUTTON_PRESS_INPUT_REVERSE',
	'SUSTAIN_PROFILE_SLIDER_POSITION_1',
	'SUSTAIN_PROFILE_SLIDER_POSITION_2',
	'SUSTAIN_PROFILE_SLIDER_POSITION_3',
	'SUSTAIN_PROFILE_SLIDER_POSITION_4',
	'SUSTAIN_PROFILE_SLIDER_POSITION_5',
	'SUSTAIN_PROFILE_SLIDER_POSITION_6',
	'SUSTAIN_PROFILE_SLIDER_POSITION_7',
	'SUSTAIN_PROFILE_SLIDER_POSITION_8',
];

type OptionType = {
	label: string;
	value: PinActionValues;
	type: 'customButtonMask' | 'customDpadMask' | 'action';
	customButtonMask: number;
	customDpadMask: number;
};

const getMask = (maskArr: { label: string; value: number }[], key: string) =>
	maskArr.find(
		({ label }) => label?.toUpperCase() === key.split('BUTTON_PRESS_')?.pop(),
	);

// Buttons can be combined into a custom combo, the same way as on the Pin Mapping page
const options: OptionType[] = SUPPORTED_ACTION_KEYS.map((key) => {
	const buttonMask = getMask(BUTTON_MASKS, key);
	const dpadMask = getMask(DPAD_MASKS, key);
	return {
		label: key,
		value: BUTTON_ACTIONS[key],
		type: buttonMask
			? 'customButtonMask'
			: dpadMask
				? 'customDpadMask'
				: 'action',
		customButtonMask: buttonMask?.value || 0,
		customDpadMask: dpadMask?.value || 0,
	};
});

const groupedOptions = [
	{
		label: 'Buttons',
		options: options.filter(({ type }) => type !== 'action'),
	},
	{
		label: 'Actions',
		options: options.filter(({ type }) => type === 'action'),
	},
];

const getSelectedOptions = (pinData: ExpansionPin) =>
	pinData.option === BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO
		? options.filter(
				({ type, customButtonMask, customDpadMask }) =>
					(type === 'customButtonMask' &&
						(pinData.customButtonMask ?? 0) & customButtonMask) ||
					(type === 'customDpadMask' &&
						(pinData.customDpadMask ?? 0) & customDpadMask),
			)
		: options.filter(({ value }) => value === pinData.option);

// Several buttons become a custom combo; picking an action replaces the selection
const toPinData = (
	selected: MultiValue<OptionType> | SingleValue<OptionType>,
): Partial<ExpansionPin> => {
	const list = (Array.isArray(selected) ? selected : [selected]).filter(
		Boolean,
	) as OptionType[];
	const noMasks = { customButtonMask: 0, customDpadMask: 0 };
	if (!list.length) return { option: BUTTON_ACTIONS.NONE, ...noMasks };

	const last = list[list.length - 1];
	if (list.length === 1 || last.type === 'action') {
		return { option: last.value, ...noMasks };
	}
	return list
		.filter(({ type }) => type !== 'action')
		.reduce(
			(pin, { customButtonMask, customDpadMask }) => ({
				option: BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO,
				customButtonMask: pin.customButtonMask | customButtonMask,
				customDpadMask: pin.customDpadMask | customDpadMask,
			}),
			{ option: BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO, ...noMasks },
		);
};

export const shiftRegisterInputScheme = {
	ShiftRegisterInputAddonEnabled: yup
		.number()
		.required()
		.label('Shift Register Input Add-On Enabled'),
	shiftRegisterInputLatchPin: yup.number().label('Latch Pin'),
	shiftRegisterInputClockPin: yup.number().label('Clock Pin'),
	shiftRegisterInputDataPin: yup.number().label('Data Pin'),
	shiftRegisterInputChainLength: yup
		.number()
		.min(1)
		.max(MAX_CHIPS)
		.label('Number of Chips'),
	shiftRegisterInputActiveHigh: yup.number().label('Input Level'),
};

export const shiftRegisterInputState = {
	ShiftRegisterInputAddonEnabled: 0,
	shiftRegisterInputLatchPin: -1,
	shiftRegisterInputClockPin: -1,
	shiftRegisterInputDataPin: -1,
	shiftRegisterInputChainLength: 1,
	shiftRegisterInputActiveHigh: 0,
};

const ShiftRegisterInput = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { buttonLabels } = useContext(AppContext);
	const { boardDefinition } = useBoardDefinition();
	const { fetchPins, pins, savePins, setPin } = useExpansionPinStore();
	const [saveMessage, setSaveMessage] = useState('');
	const { t } = useTranslation();

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const buttonNames = omit(
		getButtonLabels(buttonLabelType, swapTpShareLabels),
		['label', 'value'],
	);

	useEffect(() => {
		fetchPins();
	}, []);

	const handleSavePins = useCallback(async () => {
		try {
			await savePins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	}, [savePins]);

	const chainLength = Math.min(
		Math.max(Number(values.shiftRegisterInputChainLength) || 1, 1),
		MAX_CHIPS,
	);
	const inputs = Object.entries(pins[EXPANSION_TYPE]?.[0] ?? {}).slice(
		0,
		chainLength * INPUT_LETTERS.length,
	);

	return (
		<Section title={t('ShiftRegisterInput:header-text')}>
			<div
				id="ShiftRegisterInputAddonOptions"
				hidden={!values.ShiftRegisterInputAddonEnabled}
			>
				<div className="alert alert-info" role="alert">
					{t('ShiftRegisterInput:sub-header-text')}
				</div>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('ShiftRegisterInput:latch-pin-label')}
						name="shiftRegisterInputLatchPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.shiftRegisterInputLatchPin}
						error={errors.shiftRegisterInputLatchPin}
						isInvalid={Boolean(errors.shiftRegisterInputLatchPin)}
						onChange={handleChange}
						min={-1}
						max={boardDefinition.maxPin}
					/>
					<FormControl
						type="number"
						label={t('ShiftRegisterInput:clock-pin-label')}
						name="shiftRegisterInputClockPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.shiftRegisterInputClockPin}
						error={errors.shiftRegisterInputClockPin}
						isInvalid={Boolean(errors.shiftRegisterInputClockPin)}
						onChange={handleChange}
						min={-1}
						max={boardDefinition.maxPin}
					/>
					<FormControl
						type="number"
						label={t('ShiftRegisterInput:data-pin-label')}
						name="shiftRegisterInputDataPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.shiftRegisterInputDataPin}
						error={errors.shiftRegisterInputDataPin}
						isInvalid={Boolean(errors.shiftRegisterInputDataPin)}
						onChange={handleChange}
						min={-1}
						max={boardDefinition.maxPin}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('ShiftRegisterInput:chain-length-label')}
						name="shiftRegisterInputChainLength"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.shiftRegisterInputChainLength}
						error={errors.shiftRegisterInputChainLength}
						isInvalid={Boolean(errors.shiftRegisterInputChainLength)}
						onChange={handleChange}
					>
						{Array.from({ length: MAX_CHIPS }, (_, i) => i + 1).map((count) => (
							<option
								key={`shiftRegisterInputChainLength-${count}`}
								value={count}
							>
								{t('ShiftRegisterInput:chain-length-option', {
									count,
									inputs: count * INPUT_LETTERS.length,
								})}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('ShiftRegisterInput:input-level-label')}
						name="shiftRegisterInputActiveHigh"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.shiftRegisterInputActiveHigh}
						error={errors.shiftRegisterInputActiveHigh}
						isInvalid={Boolean(errors.shiftRegisterInputActiveHigh)}
						onChange={handleChange}
					>
						<option value={0}>{t('ShiftRegisterInput:active-low')}</option>
						<option value={1}>{t('ShiftRegisterInput:active-high')}</option>
					</FormSelect>
				</Row>
				<h2 className="fs-5">{t('ShiftRegisterInput:mapping-header')}</h2>
				<p>{t('ShiftRegisterInput:mapping-sub-header')}</p>
				<div className="shift-register-chips d-flex flex-wrap gap-4 mt-2">
					{Array.from({ length: chainLength }, (_, chip) => (
						<div
							key={`shiftRegisterInput-chip-${chip}`}
							className="shift-register-chip d-flex flex-column gap-2"
						>
							{inputs
								.slice(
									chip * INPUT_LETTERS.length,
									(chip + 1) * INPUT_LETTERS.length,
								)
								.map(([pin, pinData], index) => {
									const label = `${chip + 1}${INPUT_LETTERS[index]}`;
									return (
										<div
											key={`shiftRegisterInput-${pin}`}
											className="d-flex align-items-center gap-2"
										>
											<div
												className="d-flex flex-shrink-0"
												style={{ width: '2.5rem' }}
											>
												<label htmlFor={`shiftRegisterInput-${pin}`}>
													{label}
												</label>
											</div>
											<div className="flex-grow-1">
												<CustomSelect
													inputId={`shiftRegisterInput-${pin}`}
													isClearable
													isMulti
													isSearchable
													options={groupedOptions}
													value={getSelectedOptions(pinData)}
													getOptionLabel={(option: OptionType) => {
														const labelKey = option.label
															.split('BUTTON_PRESS_')
															.pop();
														// Fall back for actions that aren't button names
														return (
															(labelKey && buttonNames[labelKey]) ||
															t(`PinMapping:actions.${option.label}`)
														);
													}}
													onChange={(selected) =>
														setPin(EXPANSION_TYPE, 0, pin, toPinData(selected))
													}
												/>
											</div>
										</div>
									);
								})}
						</div>
					))}
				</div>
				<Button type="button" onClick={handleSavePins} className="my-4">
					{t('ShiftRegisterInput:save-mapping-label')}
				</Button>
				{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="ShiftRegisterInputAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ShiftRegisterInputAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('ShiftRegisterInputAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ShiftRegisterInput;
