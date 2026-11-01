export default {
	'header-text': 'Shift Register Input (74HC165)',
	'sub-header-text':
		'Reads extra buttons from a chain of 74HC165 shift registers. Wire SH/LD of every chip to the latch pin, CLK to the clock pin, tie CLK INH low, and connect QH of chip 1 to the data pin. Each further chip feeds its QH into the SER input of the chip before it. Power the chips from 3.3V and give every input a pull-up resistor.',
	'latch-pin-label': 'Latch Pin (SH/LD)',
	'clock-pin-label': 'Clock Pin (CLK)',
	'data-pin-label': 'Data Pin (QH)',
	'chain-length-label': 'Number of Chips',
	'chain-length-option': '{{count}} ({{inputs}} inputs)',
	'input-level-label': 'Pressed Level',
	'active-low': 'Low (button to ground)',
	'active-high': 'High (button to 3.3V)',
	'mapping-header': 'Input Mapping',
	'mapping-sub-header':
		'Inputs are named by chip and pin, so 1A is input A of the chip wired to the data pin. Pick several buttons to press them together as a combo; picking an action such as Turbo or a macro replaces the selection. Save Mapping stores these assignments; the pins and other settings above are saved with the page Save button. Changes apply after a reboot.',
	'save-mapping-label': 'Save Mapping',
};
