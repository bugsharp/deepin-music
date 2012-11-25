#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 Deepin, Inc.
#               2012 Hailong Qiu
#
# Author:     Hailong Qiu <356752238@qq.com>
# Maintainer: Hailong Qiu <356752238@qq.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gtk
import gobject

from dtk.ui.cache_pixbuf import CachePixbuf
from dtk.ui.draw import draw_pixbuf
from dtk.ui.utils import set_cursor

from widget.skin import app_theme

STATE_NORMAL = 1
STATE_HOVER = 2
STATE_PRESS = 3

class VolumeButton(gtk.Button):
    
    __gsignals__ = { 
        "volume-state-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_PYOBJECT, gobject.TYPE_PYOBJECT)),
                     }
    
    def __init__(self, value=100, lower=0, upper=100, step=5, progress_width=45):
        gtk.Button.__init__(self)
        
        # Init data.
        self.__value = value
        self.__lower = lower
        self.__upper = upper
        self.__step = step
        self.progress_width = progress_width
        self.mute_flag = False
        self.state_press_flag = False
        self.drag_flag = False
        self.drag_out_area = False
        self.hide_progress_flag = True
        self.current_progress_width = self.value_to_width(self.__value)
        self.icon_state = STATE_NORMAL
        
        # Init DPixbufs.
        self.bg_dpixbuf = app_theme.get_pixbuf("volume/bg.png")
        self.fg_dpixbuf = app_theme.get_pixbuf("volume/fg.png")
        self.point_dpixbuf = app_theme.get_pixbuf("volume/point.png")
        self.update_state_dpixbufs(self.get_state_name(self.__value))
        
        # Init Sizes.
        self.padding_x = 0
        self.padding_y = 0
        self.progress_x = 0
        self.state_icon_rect = gtk.gdk.Rectangle(
            self.padding_x, self.padding_y,
            self.normal_dpixbuf.get_pixbuf().get_width() - 2,
            self.normal_dpixbuf.get_pixbuf().get_height())
        
        self.point_width = self.point_dpixbuf.get_pixbuf().get_width()
        self.base_width = self.padding_x * 2 + self.normal_dpixbuf.get_pixbuf().get_width()        
        self.expand_width = self.base_width + self.progress_width + self.progress_x  + self.point_width
        self.default_height = self.padding_y * 2 + self.normal_dpixbuf.get_pixbuf().get_height()
        self.point_offset = self.state_icon_rect.x + self.state_icon_rect.width + self.progress_x - 2
        self.fg_offset = self.bg_offset = self.point_offset + self.point_width / 2
        
        # Init CachePixbufs
        self.bg_cache_pixbuf = CachePixbuf()
        self.fg_cache_pixbuf = CachePixbuf()
        
        # Init Events.
        self.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.connect("expose-event", self.on_expose_event)
        self.connect("scroll-event", self.on_scroll_event)
        self.connect("enter-notify-event", self.on_enter_notify_event)
        self.connect("leave-notify-event", self.on_leave_notify_event)
        self.connect("button-press-event", self.on_button_press_event)
        self.connect("motion-notify-event", self.on_motion_notify_event)
        self.connect("button-release-event", self.on_button_release_event)
        
        if self.hide_progress_flag:
            self.set_size_request(self.base_width, self.default_height)
        else:    
            self.set_size_request(self.expand_width, self.default_height)
            
    def value_to_width(self, value):    
        return value / float(self.__upper) * self.progress_width
    
    def width_to_value(self, width):
        return width / float(self.progress_width) * self.__upper 
    
    def update_state_by_value(self, emit=True):
        value = self.width_to_value(int(self.current_progress_width))
        state_name = self.get_state_name(value)
        self.update_state_dpixbufs(state_name, queue_draw=True)
        if emit:
            self.emit("volume-state-changed", self.get_value(), self.mute_flag)        
    
    def update_progress_width(self, event):
        self.current_progress_width = int(event.x - self.fg_offset)
        if self.current_progress_width < 0:
            self.current_progress_width = 0
        elif self.current_progress_width > self.progress_width:    
            self.current_progress_width = self.progress_width
        self.update_state_by_value()    
        self.queue_draw()
        
    def update_state_dpixbufs(self, name, queue_draw=False):    
        self.normal_dpixbuf = app_theme.get_pixbuf("volume/%s_normal.png" % name)
        self.hover_dpixbuf = app_theme.get_pixbuf("volume/%s_hover.png" % name)
        self.press_dpixbuf = app_theme.get_pixbuf("volume/%s_press.png" % name)
        
        if queue_draw:
            self.queue_draw()
        
    def get_state_name(self, value):
        if value == 0:
            state_name = "zero"
        elif 0 < value <= self.__upper * (1.0/3):
            state_name = "low"
        elif self.__upper * (1.0/3) < value <= self.__upper * (2.0 / 3):    
            state_name = "medium"
        else:    
            state_name = "high"
        return state_name    
    
    def on_expose_event(self, widget, event):
        cr = widget.window.cairo_create()
        rect = widget.allocation
        
        # Draw state icon.
        if self.icon_state == STATE_HOVER:
            pixbuf = self.hover_dpixbuf.get_pixbuf()
        elif self.icon_state == STATE_PRESS:    
            pixbuf = self.press_dpixbuf.get_pixbuf()
        else:    
            pixbuf = self.normal_dpixbuf.get_pixbuf()
        draw_pixbuf(cr, pixbuf, rect.x + self.padding_x, rect.y + self.padding_y)    
        
        if not self.hide_progress_flag:
            self.draw_progress_bar(cr, rect)
        return True    
        
    def draw_progress_bar(self, cr, rect):                    
        
        # Draw progressbar background.
        bg_height = self.bg_dpixbuf.get_pixbuf().get_height()
        self.bg_cache_pixbuf.scale(self.bg_dpixbuf.get_pixbuf(), self.progress_width, 
                                   bg_height)
        

        bg_y = rect.y + (rect.height - bg_height) / 2
        draw_pixbuf(cr, self.bg_cache_pixbuf.get_cache(), rect.x + self.bg_offset, bg_y)
        
        # Draw progressbar foreground.
        if self.current_progress_width > 0:
            fg_height = self.fg_dpixbuf.get_pixbuf().get_height()
            self.fg_cache_pixbuf.scale(self.fg_dpixbuf.get_pixbuf(), 
                                       int(self.current_progress_width),
                                       fg_height)
        
            fg_y = rect.y + (rect.height - fg_height) / 2
            draw_pixbuf(cr, self.fg_cache_pixbuf.get_cache(),  rect.x + self.fg_offset, fg_y)
        
        # Draw point.
        point_y = rect.y + (rect.height - self.point_dpixbuf.get_pixbuf().get_height()) / 2
        draw_pixbuf(cr, self.point_dpixbuf.get_pixbuf(), 
                    rect.x + self.point_offset + self.current_progress_width, 
                    point_y)
            
    def on_enter_notify_event(self, widget, event):
        self.hide_progress_flag = False
        self.set_size_request(self.expand_width, self.default_height)
        self.queue_draw()
        
    def hide_progressbar(self):    
        self.hide_progress_flag = True
        self.set_size_request(self.base_width, self.default_height)
        self.icon_state = STATE_NORMAL
        set_cursor(self, None)
        self.queue_draw()
    
    def on_leave_notify_event(self, widget, event):
        if self.drag_flag:
            self.drag_out_area = True
        else:    
            self.hide_progressbar()
    
    def pointer_in_state_icon(self, event):    
        if self.state_icon_rect.x <= event.x <= self.state_icon_rect.x + self.state_icon_rect.width  and \
                self.state_icon_rect.y <= event.y <= self.state_icon_rect.y + self.state_icon_rect.height:
            return True
        return False
        
    def on_button_press_event(self, widget, event):
        if self.pointer_in_state_icon(event):
            self.state_press_flag = True
            self.icon_state = STATE_PRESS
        else:    
            self.mute_flag = False
            self.update_progress_width(event)
            self.drag_flag = True
            
        self.queue_draw()
    
    def on_motion_notify_event(self, widget, event):
        if self.pointer_in_state_icon(event):
            self.icon_state = STATE_HOVER
            set_cursor(widget, None)
        else:    
            self.icon_state = STATE_NORMAL
            set_cursor(widget, gtk.gdk.HAND2)
            
        if self.drag_flag:
            self.update_progress_width(event)
        self.queue_draw()

    def on_button_release_event(self, widget, event):
        if self.drag_out_area:
            self.hide_progressbar()
        self.drag_out_area = False    
        
        if self.state_press_flag:
            if self.mute_flag:
                self.update_state_by_value()
                self.mute_flag = False
            else:    
                self.update_state_dpixbufs("mute")
                self.mute_flag = True
            self.emit("volume-state-changed", self.get_value(), self.mute_flag)    
                
        self.icon_state = STATE_NORMAL        
        self.state_press_flag = False        
        self.drag_flag = False
        self.queue_draw()
        
    def get_value(self):    
        return self.width_to_value(self.current_progress_width)
    
    def set_value(self, value):
        self.current_progress_width = self.value_to_width(value)
        self.update_state_by_value(emit=True)
        self.queue_draw()
        
    def on_scroll_event(self, widget, event):
        self.mute_flag = False
        if event.direction == gtk.gdk.SCROLL_UP:
            self.increase_value()
        elif event.direction == gtk.gdk.SCROLL_DOWN:
            self.decrease_value()
            
    def increase_value(self):    
        temp_width = self.current_progress_width
        temp_width += self.value_to_width(self.__step)
        if temp_width > self.progress_width:
            temp_width = self.progress_width
        if temp_width != self.current_progress_width:
            self.current_progress_width = temp_width
            self.update_state_by_value()
            self.queue_draw()
            
    def decrease_value(self):        
        temp_width = self.current_progress_width
        temp_width -= self.value_to_width(self.__step)
        if temp_width < 0:
            temp_width = 0
        if temp_width != self.current_progress_width:
            self.current_progress_width = temp_width
            self.update_state_by_value()
            self.queue_draw()
