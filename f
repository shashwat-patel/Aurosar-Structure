#!/usr/bin/env python3
"""
SIMPLIFIED ECU Timing Diagram Generator for Excel
Creates timing diagrams with border-based signal representation
"""

import openpyxl
from openpyxl import Workbook
from openpyxl.styles import PatternFill, Border, Side, Alignment, Font
from openpyxl.utils import get_column_letter

class SimpleTimingDiagram:
    """Super simple timing diagram generator with border-based signals"""
    
    def __init__(self):
        self.wb = Workbook()
        self.ws = self.wb.active
        self.ws.title = "Timing"
        
        # Border styles for signals
        self.thin = Side(style='thin', color='000000')
        self.medium = Side(style='medium', color='000000')
        self.thick = Side(style='thick', color='000000')
        self.dotted = Side(style='dotted', color='808080')
        
        # State colors (only for state boxes)
        self.colors = {
            'OFF': 'FF6B6B',      # Red
            'INIT': 'FFD93D',     # Yellow  
            'RUN': '6BCF7F',      # Green
            'SLEEP': 'E8E8E8',    # Light gray
            'WAKE': 'FFE66D',     # Light yellow
            'POST1': '95E1D3',    # Light cyan
            'POST2': 'F38181',    # Light red
            'LATCH': '4ECDC4',    # Cyan
            'POWERLATCH': '4ECDC4',  # Cyan
            'POSTRUN1': '95E1D3',    # Light cyan
            'POSTRUN2': 'F38181',    # Light red
            'START': 'FFD93D',       # Yellow
            'STARTUP': 'FFD93D',     # Yellow
            'POWER_OFF': 'FF6B6B',   # Red
            'RUN_MODE': '6BCF7F',    # Green
        }
        
        self.current_row = 5  # Start at row 5 to leave space for headers
        self.time_units = 50  # Total time units to show
        self.first_signal_row = 5
        
    def setup_basic_grid(self, title="Timing Diagram"):
        """Basic setup - title, MEASUREMENT header, and grid"""
        
        # MEASUREMENT label in row 2 (not row 1)
        self.ws.merge_cells('B2:AZ2')
        self.ws['B2'] = 'MEASUREMENT'
        self.ws['B2'].font = Font(size=12, bold=True)
        self.ws['B2'].alignment = Alignment(horizontal='center')
        
        # Column widths
        self.ws.column_dimensions['B'].width = 25  # Signal names
        for i in range(3, 53):  # 50 time columns
            self.ws.column_dimensions[get_column_letter(i)].width = 2.5
            
        # Time header in row 3 (every 5 units = 100ms)
        for i in range(0, self.time_units, 5):
            cell = self.ws.cell(row=3, column=3+i, value=f"{i*20}ms")
            cell.font = Font(size=9, italic=True)
            
        # Add vertical dotted grid lines
        for i in range(0, self.time_units):
            if i % 5 == 0:  # Major grid every 5 units
                for row in range(4, 50):
                    cell = self.ws.cell(row=row, column=3+i)
                    cell.border = Border(left=self.dotted)
        
        # Leave row 4 empty as spacer
        self.current_row = 5
        self.first_signal_row = 5
    
    def add(self, signal_name, pattern):
        """
        SUPER SIMPLE: Just give signal name and pattern!
        
        For digital signals:
            - Use 0 (low), 1 (high), - (don't care), X (undefined)
            - Transitions are shown with borders:
              * Rising edge: left + top borders
              * Falling edge: top + right borders
              * High level: top border only
              * Low level: bottom border only
        
        For state signals:
            - Use state names separated by spaces
        
        Examples:
            add("Power", "0000011111111111111111111111111110000000")
            add("Clock", "0101010101010101010101010101010101010101")
            add("State", "OFF OFF OFF INIT INIT RUN RUN RUN RUN SLEEP")
        """
        row = self.current_row
        self.current_row += 1
        
        # Add signal name
        self.ws.cell(row=row, column=2, value=signal_name)
        self.ws.cell(row=row, column=2).font = Font(size=10, bold=True)
        
        # Handle different pattern types
        if ' ' in pattern:
            # State pattern (words separated by spaces)
            self._add_states(row, pattern.split())
        else:
            # Digital pattern with border-based representation
            self._add_digital_with_borders(row, pattern)
            
        # Set row height
        self.ws.row_dimensions[row].height = 20
    
    def _add_digital_with_borders(self, row, pattern):
        """Add digital signal using borders for transitions"""
        prev_val = '0'  # Assume starting from low
        
        for i, char in enumerate(pattern):
            if i >= self.time_units:
                break
                
            cell = self.ws.cell(row=row, column=3+i)
            
            # Determine what borders to apply based on transition
            left_border = None
            right_border = None
            top_border = None
            bottom_border = None
            
            if char in ['0', '1']:
                # Check for transitions
                if i > 0:
                    prev_val = pattern[i-1] if i > 0 else '0'
                
                if prev_val != char:
                    # Transition detected
                    if char == '1':
                        # Rising edge: left + top borders
                        left_border = self.thin
                        top_border = self.thin
                    else:
                        # Falling edge: top + right borders
                        if i > 0:
                            # Add right border to previous cell
                            prev_cell = self.ws.cell(row=row, column=3+i-1)
                            prev_border = prev_cell.border
                            prev_cell.border = Border(
                                left=prev_border.left,
                                right=self.thin,
                                top=prev_border.top,
                                bottom=prev_border.bottom
                            )
                        top_border = self.thin
                
                # Steady state
                if char == '1':
                    # High level: top border
                    if not top_border:
                        top_border = self.thin
                else:
                    # Low level: bottom border
                    bottom_border = self.thin
            
            elif char == '-':
                # Don't care - draw dashed middle line
                top_border = self.dotted
                
            elif char == 'X':
                # Undefined - draw box
                left_border = self.dotted
                right_border = self.dotted
                top_border = self.dotted
                bottom_border = self.dotted
            
            # Apply the borders
            if left_border or right_border or top_border or bottom_border:
                cell.border = Border(
                    left=left_border,
                    right=right_border,
                    top=top_border,
                    bottom=bottom_border
                )
    
    def _add_states(self, row, states):
        """Add state machine signal from list of states"""
        current_state = states[0]
        start_col = 0
        
        for i, state in enumerate(states + ['END']):  # Add END to trigger last state
            if state != current_state or i == len(states):
                # State changed, draw the previous state
                end_col = i - 1
                
                # Merge cells for this state
                if end_col >= start_col:
                    if end_col == start_col:
                        # Single cell
                        cell = self.ws.cell(row=row, column=3+start_col)
                    else:
                        # Merge multiple cells
                        self.ws.merge_cells(
                            start_row=row, start_column=3+start_col,
                            end_row=row, end_column=3+end_col
                        )
                        cell = self.ws.cell(row=row, column=3+start_col)
                    
                    # Set state text and color
                    cell.value = current_state
                    cell.alignment = Alignment(horizontal='center', vertical='center')
                    cell.font = Font(size=9, bold=True)
                    
                    # Apply color
                    color = self.colors.get(current_state.upper(), 'FFFFFF')
                    cell.fill = PatternFill(start_color=color, end_color=color, fill_type='solid')
                    
                    # Add borders around state box
                    for col in range(3+start_col, 3+end_col+1):
                        c = self.ws.cell(row=row, column=col)
                        left = self.thin if col == 3+start_col else None
                        right = self.thin if col == 3+end_col else None
                        c.border = Border(
                            left=left,
                            right=right,
                            top=self.thin,
                            bottom=self.thin
                        )
                
                # Update for next state
                current_state = state
                start_col = i
    
    def add_box_signal(self, signal_name, start, end, label=""):
        """Add a box-style signal (like CAN/LIN Wakeup)"""
        row = self.current_row
        self.current_row += 1
        
        # Add signal name
        self.ws.cell(row=row, column=2, value=signal_name)
        self.ws.cell(row=row, column=2).font = Font(size=10, bold=True)
        
        # Draw box
        for i in range(start, end + 1):
            cell = self.ws.cell(row=row, column=3+i)
            
            left = self.thin if i == start else None
            right = self.thin if i == end else None
            top = self.thin
            bottom = self.thin
            
            cell.border = Border(left=left, right=right, top=top, bottom=bottom)
        
        # Add label if provided
        if label:
            mid = (start + end) // 2
            cell = self.ws.cell(row=row, column=3+mid, value=label)
            cell.font = Font(size=9, italic=True)
            cell.alignment = Alignment(horizontal='center', vertical='center')
        
        self.ws.row_dimensions[row].height = 20
    
    def space(self):
        """Add empty row for spacing"""
        self.current_row += 1
        
    def section(self, title):
        """Add section header"""
        row = self.current_row
        self.current_row += 1
        
        self.ws.merge_cells(f'B{row}:D{row}')
        cell = self.ws.cell(row=row, column=2, value=title)
        cell.font = Font(size=11, bold=True)
        cell.fill = PatternFill(start_color='D9D9D9', end_color='D9D9D9', fill_type='solid')
        cell.border = Border(
            left=self.medium,
            right=self.medium,
            top=self.medium,
            bottom=self.medium
        )
        
    def timing_mark(self, start_time, end_time, label):
        """Add timing annotation (like "200ms")"""
        row = self.current_row
        self.current_row += 1
        
        # Add label at midpoint
        mid = (start_time + end_time) // 2
        cell = self.ws.cell(row=row, column=3+mid, value=label)
        cell.font = Font(size=9, italic=True, color='FF0000')
        cell.alignment = Alignment(horizontal='center')
        
        # Draw line with end markers
        for i in range(start_time, end_time + 1):
            cell = self.ws.cell(row=row, column=3+i)
            if i == start_time:
                cell.border = Border(left=self.thin, top=self.thin)
            elif i == end_time:
                cell.border = Border(right=self.thin, top=self.thin)
            else:
                cell.border = Border(top=Side(style='thin', color='FF0000'))
    
    def save(self, filename="timing.xlsx"):
        """Save the file"""
        # Add main border around entire diagram
        self.add_main_border()
        
        # Remove gridlines for cleaner look
        self.ws.sheet_view.showGridLines = False
        self.wb.save(filename)
        print(f"✓ Saved: {filename}")
    
    def add_main_border(self):
        """Add thick border around entire diagram"""
        # Find the last row with content
        last_row = self.current_row - 1
        
        # Apply thick border starting from row 2 (MEASUREMENT row)
        start_row = 2
        end_row = last_row
        start_col = 2  # Column B
        end_col = 52   # Column AZ
        
        for row in range(start_row, end_row + 1):
            for col in range(start_col, end_col + 1):
                cell = self.ws.cell(row=row, column=col)
                existing = cell.border
                
                # Only apply borders on edges
                if row == start_row or row == end_row or col == start_col or col == end_col:
                    cell.border = Border(
                        left=self.thick if col == start_col else (existing.left if existing else None),
                        right=self.thick if col == end_col else (existing.right if existing else None),
                        top=self.thick if row == start_row else (existing.top if existing else None),
                        bottom=self.thick if row == end_row else (existing.bottom if existing else None)
                    )