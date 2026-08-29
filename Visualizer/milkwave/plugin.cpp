/*
  LICENSE
  -------
Copyright 2005-2013 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of Nullsoft nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
  ##########################################################################################

  case 'q':
    m_pState->m_fVideoEchoZoom /= 1.05f;
    SendPresetWaveInfoToMilkwaveRemote();
    return 0; // we processed (or absorbed) the key
  case 'Q':
    m_pState->m_fVideoEchoZoom *= 1.05f;
    SendPresetWaveInfoToMilkwaveRemote();
    return 0; // we processed (or absorbed) the key
    Order of function calls...

    When the PLUGIN launches
    ------------------------
        INITIALIZATION
            OverrideDefaults
            MyPreInitialize
            MyReadConfig
            << DirectX gets initialized at this point >>
            AllocateMyNonDx9Stuff
            AllocateMyDX9Stuff
        RUNNING
            +--> { CleanUpMyDX9Stuff + AllocateMyDX9Stuff }  // called together when user resizes window or toggles fullscreen<->windowed.
            |    MyRenderFn
            |    MyRenderUI
            |    { MyWindowProc }                            // called, between frames, on mouse/keyboard/system events.  100% threadsafe.
            +----<< repeat >>
        CLEANUP
            CleanUpMyDX9Stuff
            CleanUpMyNonDx9Stuff
            << DirectX gets uninitialized at this point >>

    When the CONFIG PANEL launches
    ------------------------------
        INITIALIZATION
            OverrideDefaults
            MyPreInitialize
            MyReadConfig
            << DirectX gets initialized at this point >>
        RUNNING
            { MyConfigTabProc }                  // called on startup & on keyboard events
        CLEANUP
            [ MyWriteConfig ]                    // only called if user clicked 'OK' to exit
            << DirectX gets uninitialized at this point >>
*/

/*
  NOTES
  -----



  To do
  -----
    -VMS VERSION:
        -based on vms 1.05, but the 'fix slow text' option has been added.
            that includes m_lpDDSText, CTextManager (m_text), changes to
            DrawDarkTranslucentBox, the replacement of all DrawText calls
            (now routed through m_text), and adding the 'fix slow text' cb
            to the config panel.

    -KILLED FEATURES:
        -vj mode

    -NEW FEATURES FOR 1.04:
            -added the following variables for per-frame scripting: (all booleans, except 'gamma')
              wave_usedots, wave_thick, wave_additive, wave_brighten
                gamma, darken_center, wrap, invert, brighten, darken, solarize
                (also, note that echo_zoom, echo_alpha, and echo_orient were already in there,
                 but weren't covered in the documentation!)
        d   -fixed: spectrum w/512 samples + 256 separation -> infinite spike
        d   -reverted dumb changes to aspect ratio stuff
        d   -reverted wave_y fix; now it's backwards, just like it's always been
                (i.e. for wave's y position, 0=bottom and 1=top, which is opposite
                to the convention in the rest of milkdrop.  decided to keep the
                'bug' so presets don't need modified.)
        d   -fixed: Krash: Inconsistency bug - pressing Escape while in the code windows
                for custom waves completely takes you out of the editing menus,
                rather than back to the custom wave menu
        d   -when editing code: fix display of '&' character
        d   -internal texture size now has a little more bias toward a finer texture,
                based on the window size, when set to 'Auto'.  (Before, for example,
                to reach 1024x1024, the window had to be 768x768 or greater; now, it
                only has to be 640x640 (25% of the way there).  I adjusted it because
                before, at in-between resolutions like 767x767, it looked very grainy;
                now it will always look nice and crisp, at any window size, but still
                won't cause too much aliasing (due to downsampling for display).
        d   -fixed: rova:
                When creating presets have commented code // in the per_pixel section when cause error in preset.
                Example nothing in per_frame and just comments in the per_pixel. EXamples on repuest I have a few.
        d   -added kill keys:
                -CTRL+K kills all running sprites
                -CTRL+T kills current song title anim
                -CTRL+Y kills current custom message
        d   -notice to sprite users:
                -in milk_img.ini, color key can't be a range anymore; it's
                    now limited to just a single color.  'colorkey_lo' and
                    'colorkey_hi' have been replaced with just one setting,
                    'colorkey'.
        d   -song titles + custom messages are working again
        ?   -fixed?: crashes on window resize [out of mem]
                -Rova: BTW the same bug as krash with the window resizing.
                -NOT due to the 'integrate w/winamp' option.
                -> might be fixed now (had forgotten to release m_lpDDSText)
        <AFTER BETA 3..>
        d   -added checkbox to config screen to automatically turn SCROLL LOCK on @ startup
        d   -added alphanumeric seeking to the playlist; while playlist is up,
                you can now press A-Z and 0-9 to seek to the next song in the playlist
                that starts with that character.
        d   -<fixed major bug w/illegal mem access on song title launches;
                could have been causing crashing madness @ startup on many systems>
        d   -<fixed bug w/saving 64x48 mesh size>
        d   -<fixed squashed shapes>
        d   -<fixed 'invert' variable>
        d   -<fixed squashed song titles + custom msgs>
        ?   -<might have fixed scroll lock stuff>
        ?   -<might have fixed crashing; could have been due to null ptr for failed creation of song title texture.>
        ?   -<also might have solved any remaining resize or exit bugs by callign SetTexture(NULL)
                in DX8 cleanup.>
        d   -<fixed sizing issues with songtitle font.>
        d   -<fixed a potentially bogus call to deallocate memory on exit, when it was cleaning up the menus.>
        d   -<fixed more scroll lock issues>
        d   -<fixed broken Noughts & Crosses presets; max # of per-frame vars was one too few, after the additions of the new built-in variables.>
        d   -<hopefully fixed waveforms>
        <AFTER BETA 4>
            -now when playlist is up, SHIFT+A-Z seeks upward (while lowercase/regular a-z seeks downward).
            -custom shapes:
                -OH MY GOD
                -increased max. # of custom shapes (and waves) from 3 to 4
                -added 'texture' option, which allows you to use the last frame as a texture on the shape
                    -added "tex_ang" and "tex_zoom" params to control the texture coords
                -each frame, custom shapes now draw BEFORE regular waveform + custom waves
                -added init + per-frame vars: "texture", "additive", "thick", "tex_ang", "tex_zoom"
            -fixed valid characters for filenames when importing/exporting custom shapes/waves;
                also, it now gives error messages on error in import/export.
            -cranked max. meshsize up to 96x72
            -Krash, Rova: now the 'q' variables, as modified by the preset per-frame equations, are again
                readable by the custom waves + custom shapes.  Sorry about that.  Should be the end of the
                'q' confusion.
            -added 'meshx' and 'meshy' [read-only] variables to the preset init, per-frame, and per-pixel
                equations (...and inc'd the size of the global variable pool by 2).
            -removed t1-t8 vars for Custom Shapes; they were unnecessary (since there's no per-point code there).
            -protected custom waves from trying to draw when # of sample minus the separation is < 2
                (or 1 if drawing with dots)
            -fixed some [minor] preset-blending bugs in the custom wave code
            -created a visual map for the flow of values for the q1-q8 and t1-t8 variables:
                q_and_t_vars.gif (or something).
            -fixed clipping of onscreen text in low-video-memory situations.  Now, if there isn't enough
                video memory to create an offscreen texture that is at least 75% of the size of the
                screen (or to create at least a 256x256 one), it won't bother using one, and will instead draw text directly to the screen.
                Otherwise, if the texture is from 75%-99% of the screen size, text will now at least
                appear in the correct position on the screen so that it will be visible; this will mean
                that the right- and bottom-aligned text will no longer be fully right/bottom-aligned
                to the edge of the screen.
            -fixed blurry text
            -VJ mode is partially restored; the rest will come with beta 7 or the final release.  At the time of beta 6, VJ mode still has some glitches in it, but I'm working on them.  Most notably, it doesn't resize the text image when you resize the window; that's next on my list.
        <AFTER BETA 6:>
            -now sprites can burn-in on any frame, not just on the last frame.
                set 'burn' to one (in the sprite code) on any frame to make it burn in.
                this will break some older sprites, but it's super easy to fix, and
                I think it's worth it. =)  thanks to papaw00dy for the suggestion!
            -fixed the asymptotic-value bug with custom waves using spectral data & having < 512 samples (thanks to telek's example!)
            -fixed motion vectors' reversed Y positioning.
            -fixed truncation ("...") of long custom messages
            -fixed that pesky bug w/the last line of code on a page
            -fixed the x-positioning of custom waves & shapes.  Before, if you were
                saving some coordinates from the preset's per-frame equations (say in q1 and q2)
                and then you set "x = q1; y = q2;" in a custom shape's per-frame code
                (or in a custom wave's per-point code), the x position wouldn't really be
                in the right place, because of aspect ratio multiplications.  Before, you had
                to actually write "x = (q1-0.5)*0.75 + 0.5; y = q2;" to get it to line up;
                now it's fixed, though, and you can just write "x = q1; y = q2;".
            -fixed some bugs where the plugin start up, in windowed mode, on the wrong window
                (and hence run super slow).
            -fixed some bugs w/a munged window frame when the "integrate with winamp" option
                was checked.
        <AFTER BETA 7:>
            -preset ratings are no longer read in all at once; instead, they are scanned in
                1 per frame until they're all in.  This fixes the long pauses when you switch
                to a directory that has many hundreds of presets.  If you want to switch
                back to the old way (read them all in at once), there is an option for it
                in the config panel.
            -cranked max. mesh size up to 128x96
            -fixed bug in custom shape per-frame code, where t1-t8 vars were not
                resetting, at the beginning of each frame, to the values that they had
                @ the end of the custom shape init code's execution.
            -
            -
            -


        beta 2 thread: http://forums.winamp.com/showthread.php?threadid=142635
        beta 3 thread: http://forums.winamp.com/showthread.php?threadid=142760
        beta 4 thread: http://forums.winamp.com/showthread.php?threadid=143500
        beta 6 thread: http://forums.winamp.com/showthread.php?threadid=143974
        (+read about beat det: http://forums.winamp.com/showthread.php?threadid=102205)

@       -code editing: when cursor is on 1st posn. in line, wrong line is highlighted!?
        -requests:
            -random sprites (...they can just write a prog for this, tho)
            -Text-entry mode.
                -Like your favorite online game, hit T or something to enter 'text entry' mode. Type a message, then either hit ESC to clear and cancel text-entry mode, or ENTER to display the text on top of the vis. Easier for custom messages than editing the INI file (and probably stopping or minimizing milkdrop to do it) and reloading.
                -OR SKIP IT; EASY TO JUST EDIT, RELOAD, AND HIT 00.
            -add 'AA' parameter to custom message text file?
        -when mem is low, fonts get kicked out -> white boxes
            -probably happening b/c ID3DXFont can't create a
             temp surface to use to draw text, since all the
             video memory is gobbled up.
*       -add to installer: q_and_t_vars.gif
*       -presets:
            1. pick final set
                    a. OK-do a pass weeding out slow presets (crank mesh size up)
                    b. OK-do 2nd pass; rate them & delete crappies
                    c. OK-merge w/set from 1.03; check for dupes; delete more suckies
            2. OK-check for cpu-guzzlers
            3. OK-check for big ones (>= 8kb)
            4. check for ultra-spastic-when-audio-quiet ones
            5. update all ratings
            6. zip 'em up for safekeeping
*       -docs:
                -link to milkdrop.co.uk
                -preset authoring:
                    -there are 11 variable pools:
                        preset:
                            a) preset init & per-frame code
                            b) preset per-pixel code
                        custom wave 1:
                            c) init & per-frame code
                            d) per-point code
                        custom wave 2:
                            e) init & per-frame code
                            f) per-point code
                        custom wave 3:
                            g) init & per-frame code
                            h) per-point code
                        i) custom shape 1: init & per-frame code
                        j) custom shape 2: init & per-frame code
                        k) custom shape 3: init & per-frame code

                    -all of these have predefined variables, the values of many of which
                        trickle down from init code, to per-frame code, to per-pixel code,
                        when the same variable is defined for each of these.
                    -however, variables that you define ("my_var = 5;") do NOT trickle down.
                        To allow you to pass custom values from, say, your per-frame code
                        to your per-pixel code, the variables q1 through q8 were created.
                        For custom waves and custom shapes, t1 through t8 work similarly.
                    -q1-q8:
                        -purpose: to allow [custom] values to carry from {the preset init
                            and/or per-frame equations}, TO: {the per-pixel equations},
                            {custom waves}, and {custom shapes}.
                        -are first set in preset init code.
                        -are reset, at the beginning of each frame, to the values that
                            they had at the end of the preset init code.
                        -can be modified in per-frame code...
                            -changes WILL be passed on to the per-pixel code
                            -changes WILL pass on to the q1-q8 vars in the custom waves
                                & custom shapes code
                            -changes will NOT pass on to the next frame, though;
                                use your own (custom) variables for that.
                        -can be modified in per-pixel code...
                            -changes will pass on to the next *pixel*, but no further
                            -changes will NOT pass on to the q1-q8 vars in the custom
                                waves or custom shapes code.
                            -changes will NOT pass on to the next frame, after the
                                last pixel, though.
                        -CUSTOM SHAPES: q1-q8...
                            -are readable in both the custom shape init & per-frame code
                            -start with the same values as q1-q8 had at the end of the *preset*
                                per-frame code, this frame
                            -can be modified in the init code, but only for a one-time
                                pass-on to the per-frame code.  For all subsequent frames
                                (after the first), the per-frame code will get the q1-q8
                                values as described above.
                            -can be modified in the custom shape per-frame code, but only
                                as temporary variables; the changes will not pass on anywhere.
                        -CUSTOM WAVES: q1-q8...
                            -are readable in the custom wave init, per-frame, and per-point code
                            -start with the same values as q1-q8 had at the end of the *preset*
                                per-frame code, this frame
                            -can be modified in the init code, but only for a one-time
                                pass-on to the per-frame code.  For all subsequent frames
                                (after the first), the per-frame code will get the q1-q8
                                values as described above.
                            -can be modified in the custom wave per-frame code; changes will
                                pass on to the per-point code, but that's it.
                            -can be modified in the per-point code, and the modified values
                                will pass on from point to point, but won't carry beyond that.
                        -CUSTOM WAVES: t1-t8...
                            -allow you to generate & save values in the custom wave init code,
                                that can pass on to the per-frame and, more sigificantly,
                                per-point code (since it's in a different variable pool).
                            -...



                        !-whatever the values of q1-q8 were at the end of the per-frame and per-pixel
                            code, these are copied to the q1-q8 variables in the custom wave & custom
                            shape code, for that frame.  However, those values are separate.
                            For example, if you modify q1-q8 in the custom wave #1 code, those changes
                            will not be visible anywhere else; if you modify q1-q8 in the custom shape
                            #2 code, same thing.  However, if you modify q1-q8 in the per-frame custom
                            wave code, those modified values WILL be visible to the per-point custom
                            wave code, and can be modified within it; but after the last point,
                            the values q1-q8 will be discarded; on the next frame, in custom wave #1
                            per-frame code, the values will be freshly copied from the values of the
                            main q1-q8 after the preset's per-frame and per-point code have both been
                            executed.
                    -monitor:
                        -can be read/written in preset init code & preset per-frame code.
                        -not accessible from per-pixel code.
                        -if you write it on one frame, then that value persists to the next frame.
                    -t1-t8:
                        -
                        -
                        -
                -regular docs:
                    -put in the stuff recommended by VMS (vidcap, etc.)
                    -add to troubleshooting:
                        1) desktop mode icons not appearing?  or
                           onscreen text looking like colored boxes?
                             -> try freeing up some video memory.  lower your res; drop to 16 bit;
                                etc.  TURN OFF AUTO SONGTITLES.
                        1) slow desktop/fullscr mode?  -> try disabling auto songtitles + desktop icons.
                            also try reducing texsize to 256x256, since that eats memory that the text surface could claim.
                        2)
                        3)
        *   -presets:
                -add new
                -fix 3d presets (bring gammas back down to ~1.0)
                -check old ones, make sure they're ok
                    -"Rovastar - Bytes"
                    -check wave_y
        *   -document custom waves & shapes
        *   -slow text is mostly fixed... =(
                -desktop icons + playlist both have begin/end around them now, but in desktop mode,
                 if you bring up playlist or Load menu, fps drops in half; press Esc, and fps doesn't go back up.
            -
            -
            -
        -DONE / v1.04:
            -updated to VMS 1.05
                -[list benefits...]
                -
                -
            -3d mode:
                a) SWAPPED DEFAULT L/R LENS COLORS!  All images on the web are left=red, right=blue!
                b) fixed image display when viewing a 3D preset in a non-4:3 aspect ratio window
                c) gamma now works for 3d presets!  (note: you might have to update your old presets.
                        if they were 3D presets, the gamma was ignored and 1.0 was used; now,
                        if gamma was >1.0 in the old preset, it will probably appear extremely bright.)
                d) added SHIFT+F9 and CTRL+C9 to inc/dec stereo separation
                e) added default stereo separation to config panel
            -cranked up the max. mesh size (was 48x36, now 64x48) and the default mesh size
                (was 24x18, now 32x24)
            -fixed aspect ratio for final display
            -auto-texsize is now computed slightly differently; for vertically or horizontally-stretched
                windows, the texsize is now biased more toward the larger dimension (vs. just the
                average).
            -added anisotropic filtering (for machines that support it)
            -fixed bug where the values of many variables in the preset init code were not set prior
                to execution of the init code (e.g. time, bass, etc. were all broken!)
            -added various preset blend effects.  In addition to the old uniform fade, there is
                now a directional wipe, radial wipe, and plasma fade.
            -FIXED SLOW TEXT for DX8 (at least, on the geforce 4).
                Not using DT_RIGHT or DT_BOTTOM was the key.


        -why does text kill it in desktop mode?
        -text is SLOOW
        -to do: add (and use) song title font + tooltip font
        -re-test: menus, text, boxes, etc.
        -re-test: TIME
        -testing:
            -make sure sound works perfectly.  (had to repro old pre-vms sound analysis!)
            -autogamma: currently assumes/requires that GetFrame() resets to 0 on a mode change
                (i.e. windowed -> fullscreen)... is that the case?
            -restore motion vectors
            -
            -
        -restore lost surfaces
        -test bRedraw flag (desktop mode/paused)
        -search for //? in milkdropfs.cpp and fix things

        problem: no good soln for VJ mode
        problem: D3DX won't give you solid background for your text.
            soln: (for later-) create wrapper fn that draws w/solid bkg.

        SOLN?: use D3DX to draw all text (plugin.cpp stuff AND playlist);
        then, for VJ mode, create a 2nd DxContext
        w/its own window + windowproc + fonts.  (YUCK)
    1) config panel: test, and add WM_HELP's (copy from tooltips)
    2) keyboard input: test; and...
        -need to reset UI_MODE when playlist is turned on, and
        -need to reset m_show_playlist when UI_MODE is changed.  (?)
        -(otherwise they can both show @ same time and will fight
            for keys and draw over each other)
    3) comment out most of D3D stuff in milkdropfs.cpp, and then
        get it running w/o any milkdrop, but with text, etc.
    4) sound

  Issues / To Do Later
  --------------------
    1) sprites: color keying stuff probably won't work any more...
    2) scroll lock: pull code from Monkey
    3) m_nGridY should not always be m_nGridX*3/4!
    4) get solid backgrounds for menus, waitstring code, etc.
        (make a wrapper function!)

    99) at end: update help screen

  Things that will be different
  -----------------------------
    1) font sizes are no longer relative to size of window; they are absolute.
    2) 'don't clear screen at startup' option is gone
    3) 'always on top' option is gone
    4) text will not be black-on-white when an inverted-color preset is showing

                -VJ mode:
                    -notes
                        1. (remember window size/pos, and save it from session to session?  nah.)
                        2. (kiv: scroll lock)
                        3. (VJ window + desktop mode:)
                                -ok w/o VJ mode
                                -w/VJ mode, regardless of 'fix slow text' option, probs w/focus;
                                    click on vj window, and plugin window flashes to top of Z order!
                                -goes away if you comment out 1st call to PushWindowToJustBeforeDesktop()...
                                -when you disable PushWindowToJustBeforeDesktop:
                                    -..and click on EITHER window, milkdrop jumps in front of the taskbar.
                                    -..and click on a non-MD window, nothing happens.
                                d-FIXED somehow, magically, while fixing bugs w/true fullscreen mode!
                        4. (VJ window + true fullscreen mode:)
                                d-make sure VJ window gets placed on the right monitor, at startup,
                                    and respects taskbar posn.
                                d-bug - start in windowed mode, then dbl-clk to go [true] fullscreen
                                    on 2nd monitor, all with VJ mode on, and it excepts somewhere
                                    in m_text.DrawNow() in a call to DrawPrimitive()!
                                    FIXED - had to check m_vjd3d8_device->TestCooperativeLevel
                                    each frame, and destroy/reinit if device needed reset.
                                d-can't resize VJ window when grfx window is running true fullscreen!
                                    -FIXED, by dropping the Sleep(30)/return when m_lost_focus
                                        was true, and by not consuming WM_NCACTIVATE in true fullscreen
                                        mode when m_hTextWnd was present, since DX8 doesn't do its
                                        auto-minimize thing in that case.



========================================================================================================
SPOUT :

  Credit to psilocin@openmailbox.org for the original idea to convert MilkDrop for Spout output

  22.10.14 - changed from Ctrl-Z on and off to default Spout output when the plugin starts
         and Ctrl-Z to disable and enable while it is running. Otherwise Spout has to be re-enabled
         every time another track is selected.
  30.10.14 - changed from Glut to pixelformat and OpenGL context creation
  31.10.14 - changed initialization section to renderframe to ensure correct frame size
       - added Ctrl-D user selection of DirectX mode
       - flag bUseDX11 to select either DirectX 9 or DirectX 11
       - saved DX mode flag in configuration file
  05.11.14 - Included Spout options in the Visualization configuration control panel
        Options -> Visualizatiosn -> Configure Plugin
        MORE SETTINGS tab
          Enable Spout output
          Enable Spout DirectX 11 mode
        Settings are saved with OK
       - retained Ctrl-Z for spout on / off while the Visualizer is running
       - included Ctrl-D to change from DirectX 9 to DirectX 11
         (this might be removed in a future release if it gives trouble)
         The selected settings are saved when the Visualizer is stopped.
  25.04.15 - Changed Spout SDK from graphics auto detection to set DirectX mode to optional installer
       - Recompile for dual DX option installer
  17.06.15 - User observation that custom messages do not work.
         This is isolated to "RenderStringToTitleTexture" and seems to be related to
         generating the fonts from GDI to DX9. Not sure of the reason. Could be DX9 libraries.
         As a a workaround, custom message rendering is replaced with the same as used for
         title animation which works OK. The limitation is that this gives a fixed font,
         but the colour should come out the same as in the custom message setup file.
  07.07.15 - Recompile for 2.004 release
  15.09.15 - Recompile for 2.005 release - revised memoryshare SDK
  08.11.15 - removed directX9/directX11 option for 2.005
       - OpenSender and milkdropfs.cpp - removed XRGB format option and always send as ARGB
  12.11.18 - Removed DX11 (bUseDX11) option from plugin - test for user DX9 selection instead
  02.12.18 - Rebuild VS2017 /MT with VS2010 100 toolset - Spout 2.007
         (VS2017 140 toolset does not work)


  03.12.18 - Started modifications to the BeatDrop project (not back-compatible)
         BeatDrop name, versioning and authoring by Maxim Volskiy retained
         Use the VJ console for help and text output.
         Output resolution is 1920x1080 at start
         Resolution can subsequently be changed by resizing the render window
         The render window can be hidden with the F12 key
         and the VJ console can be minimized when not being used
         without affecting Spout output.
  04.12.18 - Monitor dpi awareness for scaled displays
         Reset help or menu text when activating either of them
         Disable minimize and maximize
         Use SpoutLibrary instead of Spout SDK source files
         Cleanup
         Rebuild VS2017 /MT with Visual Studio 2017 toolset (v141)
    05.12.18 : Create GitHub fork and update. Publish release 1001.
               Bring up the VJ console for F1 help if it has been minimised
         PluginShell.cpp - remove maximize from VJ window
  21.12.18 : Update SpoutLibrary - Version 2.007
  03.01.19 : Rebuild SpoutLibrary
         Rebuild VS2017 /MT with Visual Studio 2017 toolset (v141)
  16.01.19   TODO : Bug when resizing - re-creates the sender with a new name.
  29.04.19   Noted : setting  Monitor dpi awareness uusing Manifest tool compiler option
         results in blurry text for the console window. Retained SetProcessDpiAwareness
         Noted : warning 'Zc:forScope-' - deprecated for VS2017
         For future Visual Studio compilers, might need changes throughout for loops
  31.05.19   Rebuild with revised 2.007 SpoutLibrary
         VS2017 /MT with Visual Studio 2017 toolset (v141), Windows SDK 10.0.17763.0

  03.10.19   Change from DX9 to DX9EX

    Credit to Patrick Pomerleau of Nest Immersion (http://nestimmersion.ca/)

         Search on "DX9EX" for the changes.
          MilkwaveVisualizer.cpp
          milkdropfs.cpp
          pluginshell.cpp
          pluginshell.h
          dxcontext.cpp
          dxcontext.h

         - Set application to use Spout DX9 mode
         - Add function : IDirect3D9Ex* CPluginShell::GetDX9object()
         - OpenSender - set Spout to use the application DX9EX object and device
         - Use new surface copy function WriteDX9surface rather than CPU pixel copy.
  05.10.19   - Lock resolution to 1920x1080
         - Retain VJ console for standalone
         - Remove close button from VJ console
         - Add : F8 - copy Winamp Milkdrop generated config file
  08.10.19   - Change window resolution to 1280x720
           Retain output resolution 1920x1080
         - Modify WriteDX9surface and SetDX9device - see milkdropfs and plugin.cpp
           Modify corresponding Spout SDK and SpoutLibrary functions
         - Rebuild VS2017 /MT Win32 with modified 2.007 SpoutLibrary
           Search on "// SPOUT DX9EX" for changes
  29.10.19   - MilkwaveVisualizer.cpp
             Change dpi awareness to use SetProcessDpiAwarenessContext
             for Windows 7 compatibility
           Remove #include <ShellScalingApi.h> and #pragma comment(lib, "shcore.lib")

    15.05.23   - Change from SpoutLibrary to SpoutDX9 support class
                 Changed files :
                   vis_milk2\plugin.cpp
                   vis_milk2\plugin.h
                   vis_milk2\MilkwaveVisualizer.cpp
                   vis_milk2\milkdropfs.cpp
                   vis_milk2\pluginshell.h
                 nseel2/nseel-compiler.c
                   remove "floor" intrinsic re-definition
                 Add spoutDX9 folder and files
                 Add spoutDX9 filter to project
                 Project > Properties > Include Directories
                 Specifically include $(WindowsSDK_IncludePath) before $(DXSDK_DIR)Include
                 to prevent macro re-definitions with Windows Kits\10 together with
                 Microsoft DirectX SDK (June 2010)
                 Rebuild Visual Studio 2022 Release/Win32
    25.09.23     MyWindowProc - WM_KEYDOWN default - fall through
    26.09.23     SendDX9surface - add update flag to allow a fixed sender size if false
                 SpoutDX9.cpp/CheckDX9sender - CreateSharedDX9Texture to new width and height for size change


*/

#include "plugin.h"
#include "../audio/audiobuf.h"
#include "pipe_server.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <assert.h>
#include <locale.h>
#include <process.h>  // for beginthread, etc.
#include <shellapi.h>
#include <strsafe.h>
#include <Windows.h>
#include "AutoCharFn.h"
#include <sstream>

#include <dwmapi.h>  // Link with Dwmapi.lib
#pragma comment(lib, "dwmapi.lib")

// Define custom message IDs
#ifndef FRAND
#define FRAND ((rand() % 7381) / 7380.0f)
#endif
#ifndef clamp
#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))
#endif

int ToggleFPSNumPressed = 7;  // Default is Unlimited FPS.
int HardcutMode = 0;
float timetick = 0;
float timetick2 = 0;
float TimeToAutoLockPreset = 0;
int beatcount;
bool TranspaMode = false;
int NumTotalPresetsLoaded = 0;
bool AutoLockedPreset = false;
uint64_t LastSentMilkwaveMessage = 0;

// For Sample Rate auto-detection
#include <windows.h>
#include <mmdeviceapi.h>
#include <propsys.h>
#include <functiondiscoverykeys_devpkey.h>
#include "../audio/log.h"
#include "AMDDetection.h"
#include <cstdint>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "propsys.lib")
//

void NSEEL_HOSTSTUB_EnterMutex() {}
void NSEEL_HOSTSTUB_LeaveMutex() {}

#ifdef NS_EEL2
void NSEEL_VM_resetvars(NSEEL_VMCTX ctx) {
  NSEEL_VM_freeRAM(ctx);
  NSEEL_VM_remove_all_nonreg_vars(ctx);
}
#endif

// note: these must match layouts in support.h!!
D3DVERTEXELEMENT9 g_MyVertDecl[] =
    {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        {0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
        D3DDECL_END()};
D3DVERTEXELEMENT9 g_WfVertDecl[] =
    {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        D3DDECL_END()};
D3DVERTEXELEMENT9 g_SpriteVertDecl[] =
    {
        // matches D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        {0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        D3DDECL_END()};

// extern CSoundData*   pg_sound;	// declared in main.cpp
extern CPlugin g_plugin;  // declared in main.cpp (note: was 'pg')

// from support.cpp:
extern bool g_bDebugOutput;
extern bool g_bDumpFileCleared;

// for __UpdatePresetList:
volatile HANDLE g_hThread;         // only r/w from our MAIN thread
volatile bool g_bThreadAlive;      // set true by MAIN thread, and set false upon exit from 2nd thread.
volatile int g_bThreadShouldQuit;  // set by MAIN thread to flag 2nd thread that it wants it to exit.
CRITICAL_SECTION g_cs;
CRITICAL_SECTION g_csRemoteMessage;  // for thread-safe remote messaging

#define IsAlphabetChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define IsAlphanumericChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '.')
#define IsNumericChar(x) (x >= '0' && x <= '9')

const unsigned char LC2UC[256] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    255,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    91,
    92,
    93,
    94,
    95,
    96,
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    128,
    129,
    130,
    131,
    132,
    133,
    134,
    135,
    136,
    137,
    138,
    139,
    140,
    141,
    142,
    143,
    144,
    145,
    146,
    147,
    148,
    149,
    150,
    151,
    152,
    153,
    154,
    155,
    156,
    157,
    158,
    159,
    160,
    161,
    162,
    163,
    164,
    165,
    166,
    167,
    168,
    169,
    170,
    171,
    172,
    173,
    174,
    175,
    176,
    177,
    178,
    179,
    180,
    181,
    182,
    183,
    184,
    185,
    186,
    187,
    188,
    189,
    190,
    191,
    192,
    193,
    194,
    195,
    196,
    197,
    198,
    199,
    200,
    201,
    202,
    203,
    204,
    205,
    206,
    207,
    208,
    209,
    210,
    211,
    212,
    213,
    214,
    215,
    216,
    217,
    218,
    219,
    220,
    221,
    222,
    223,
    224,
    225,
    226,
    227,
    228,
    229,
    230,
    231,
    232,
    233,
    234,
    235,
    236,
    237,
    238,
    239,
    240,
    241,
    242,
    243,
    244,
    245,
    246,
    247,
    248,
    249,
    250,
    251,
    252,
    253,
    254,
    255,
};

/*
 * Copies the given string TO the clipboard.
 */
void copyStringToClipboardA(const char* source) {
  if (!OpenClipboard(NULL))
    return;

  EmptyClipboard();
  HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, (lstrlenA(source) + 1) * sizeof(char));
  if (!clipbuffer) {
    CloseClipboard();
    return;
  }
  char* buffer = (char*)GlobalLock(clipbuffer);
  if (!buffer) {
    GlobalFree(clipbuffer);
    CloseClipboard();
    return;
  }
  lstrcpyA(buffer, source);
  GlobalUnlock(clipbuffer);
  if (!SetClipboardData(CF_TEXT, clipbuffer))
    GlobalFree(clipbuffer);
  CloseClipboard();
}

void copyStringToClipboardW(const wchar_t* source) {
  if (!OpenClipboard(NULL))
    return;

  EmptyClipboard();
  HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, (lstrlenW(source) + 1) * sizeof(wchar_t));
  if (!clipbuffer) {
    CloseClipboard();
    return;
  }
  wchar_t* buffer = (wchar_t*)GlobalLock(clipbuffer);
  if (!buffer) {
    GlobalFree(clipbuffer);
    CloseClipboard();
    return;
  }
  lstrcpyW(buffer, source);
  GlobalUnlock(clipbuffer);
  if (!SetClipboardData(CF_UNICODETEXT, clipbuffer))
    GlobalFree(clipbuffer);
  CloseClipboard();
}

/*
 * Suppose there is a string on the clipboard.
 * This function copies it FROM there.
 * Returns a static buffer with clipboard contents, or empty string on failure.
 */
char* getStringFromClipboardA() {
  static char s_emptyA[1] = {0};
  static char s_clipA[64000];
  s_clipA[0] = 0;

  if (!OpenClipboard(NULL))
    return s_emptyA;

  HANDLE hData = GetClipboardData(CF_TEXT);
  if (!hData) {
    CloseClipboard();
    return s_emptyA;
  }
  char* buffer = (char*)GlobalLock(hData);
  if (!buffer) {
    CloseClipboard();
    return s_emptyA;
  }
  lstrcpynA(s_clipA, buffer, sizeof(s_clipA));
  GlobalUnlock(hData);
  CloseClipboard();
  return s_clipA;
}

wchar_t* getStringFromClipboardW() {
  static wchar_t s_emptyW[1] = {0};
  static wchar_t s_clipW[64000];
  s_clipW[0] = 0;

  if (!OpenClipboard(NULL))
    return s_emptyW;

  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (!hData) {
    CloseClipboard();
    return s_emptyW;
  }
  wchar_t* buffer = (wchar_t*)GlobalLock(hData);
  if (!buffer) {
    CloseClipboard();
    return s_emptyW;
  }
  lstrcpynW(s_clipW, buffer, sizeof(s_clipW) / sizeof(wchar_t));
  GlobalUnlock(hData);
  CloseClipboard();
  return s_clipW;
}

void ConvertCRsToLFCA(const char* src, char* dst) {
  while (*src) {
    char ch = *src;
    if (*src == 13 && *(src + 1) == 10) {
      *dst++ = LINEFEED_CONTROL_CHAR;
      src += 2;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = 0;
}

void ConvertCRsToLFCW(const wchar_t* src, wchar_t* dst) {
  while (*src) {
    wchar_t ch = *src;
    if (*src == 13 && *(src + 1) == 10) {
      *dst++ = LINEFEED_CONTROL_CHAR;
      src += 2;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = 0;
}

void ConvertLFCToCRsA(const char* src, char* dst) {
  while (*src) {
    char ch = *src;
    if (*src == LINEFEED_CONTROL_CHAR) {
      *dst++ = 13;
      *dst++ = 10;
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = 0;
}

void ConvertLFCToCRsW(const wchar_t* src, wchar_t* dst) {
  while (*src) {
    wchar_t ch = *src;
    if (*src == LINEFEED_CONTROL_CHAR) {
      *dst++ = 13;
      *dst++ = 10;
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = 0;
}

int mystrcmpiW(const wchar_t* s1, const wchar_t* s2) {
  // returns  1 if s1 comes before s2
  // returns  0 if equal
  // returns -1 if s1 comes after s2
  // treats all characters/symbols by their ASCII values,
  //    except that it DOES ignore case.

  int i = 0;

  while (LC2UC[s1[i]] == LC2UC[s2[i]] && s1[i] != 0)
    i++;

  // FIX THIS!

  if (s1[i] == 0 && s2[i] == 0)
    return 0;
  else if (s1[i] == 0)
    return -1;
  else if (s2[i] == 0)
    return 1;
  else
    return (LC2UC[s1[i]] < LC2UC[s2[i]]) ? -1 : 1;
}

bool ReadFileToString(const wchar_t* szBaseFilename, char* szDestText, int nMaxBytes, bool bConvertLFsToSpecialChar) {
  wchar_t szFile[MAX_PATH];
  swprintf(szFile, L"%s%s", g_plugin.m_szMilkdrop2Path, szBaseFilename);

  // read in all chars.  Replace char combos:  { 13;  13+10;  10 } with LINEFEED_CONTROL_CHAR, if bConvertLFsToSpecialChar is true.
  FILE* f = _wfopen(szFile, L"rb");
  if (!f) {
    wchar_t buf[1024], title[64];
    swprintf(buf, wasabiApiLangString(IDS_UNABLE_TO_READ_DATA_FILE_X), szFile);
    g_plugin.dumpmsg(buf);
    MessageBoxW(NULL, buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }
  int len = 0;
  int x;
  char prev_ch = 0;
  while ((x = fgetc(f)) >= 0 && len < nMaxBytes - 4) {
    char orig_ch = (char)x;
    char ch = orig_ch;
    bool bSkipChar = false;
    if (bConvertLFsToSpecialChar) {
      if (ch == 10) {
        if (prev_ch == 13)
          bSkipChar = true;
        else
          ch = LINEFEED_CONTROL_CHAR;
      } else if (ch == 13)
        ch = LINEFEED_CONTROL_CHAR;
    }

    if (!bSkipChar)
      szDestText[len++] = ch;
    prev_ch = orig_ch;
  }
  szDestText[len] = 0;
  szDestText[len++] = ' ';  // make sure there is some whitespace after
  fclose(f);
  return true;
}

// these callback functions are called by menu.cpp whenever the user finishes editing an eval_ expression.
void OnUserEditedPerFrame(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_PRESET_CODE, 0);
}

void OnUserEditedPerPixel(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_PRESET_CODE, 0);
}

void OnUserEditedPresetInit(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_PRESET_CODE, 1);
}

void OnUserEditedWavecode(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_WAVE_CODE, 0);
}

void OnUserEditedWavecodeInit(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_WAVE_CODE, 1);
}

void OnUserEditedShapecode(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_SHAPE_CODE, 0);
}

void OnUserEditedShapecodeInit(LPARAM param1, LPARAM param2) {
  g_plugin.m_pState->RecompileExpressions(RECOMPILE_SHAPE_CODE, 1);
}

void OnUserEditedWarpShaders(LPARAM param1, LPARAM param2) {
  g_plugin.m_bNeedRescanTexturesDir = true;
  g_plugin.ClearErrors(ERR_PRESET);
  if (g_plugin.m_nMaxPSVersion == 0)
    return;
  if (!g_plugin.RecompilePShader(g_plugin.m_pState->m_szWarpShadersText, &g_plugin.m_shaders.warp, SHADER_WARP, false, g_plugin.m_pState->m_nWarpPSVersion, false)) {
    // switch to fallback
    g_plugin.m_fallbackShaders_ps.warp.ptr->AddRef();
    g_plugin.m_fallbackShaders_ps.warp.CT->AddRef();
    g_plugin.m_shaders.warp = g_plugin.m_fallbackShaders_ps.warp;
  }
}

void OnUserEditedCompShaders(LPARAM param1, LPARAM param2) {
  g_plugin.m_bNeedRescanTexturesDir = true;
  g_plugin.ClearErrors(ERR_PRESET);
  if (g_plugin.m_nMaxPSVersion == 0)
    return;
  if (!g_plugin.RecompilePShader(g_plugin.m_pState->m_szCompShadersText, &g_plugin.m_shaders.comp, SHADER_COMP, false, g_plugin.m_pState->m_nCompPSVersion, false)) {
    // switch to fallback
    g_plugin.m_fallbackShaders_ps.comp.ptr->AddRef();
    g_plugin.m_fallbackShaders_ps.comp.CT->AddRef();
    g_plugin.m_shaders.comp = g_plugin.m_fallbackShaders_ps.comp;
  }
}

// Modify the help screen text here.
// Watch the # of lines, though; if there are too many, they will get cut off;
//   and watch the length of the lines, since there is no wordwrap.
// A good guideline: your entire help screen should be visible when fullscreen
//   @ 640x480 and using the default help screen font.
wchar_t* g_szHelp = 0;
wchar_t* g_szHelp_Page2 = 0;
int g_szHelp_W = 0;

// this is for integrating modern skins (with their Random button)
// and having it match our Scroll Lock (preset lock) state...
#define IPC_CB_VISRANDOM 628

//----------------------------------------------------------------------

//----------------------------------------------------------------------

void ConvertLLCto1310(char* d, const char* s) {
  // src and dest can NOT be the same pointer.
  assert(s != d);

  while (*s) {
    if (*s == LINEFEED_CONTROL_CHAR) {
      *d++ = 13;
      *d++ = 10;
    } else {
      *d++ = *s;
    }
    s++;
  };
  *d = 0;
}

void StripComments(char* str) {
  if (!str || !str[0] || !str[1])
    return;

  char c0 = str[0];
  char c1 = str[1];
  char* dest = str;
  char* p = &str[1];
  bool bIgnoreTilEndOfLine = false;
  bool bIgnoreTilCloseComment = false;  // this one takes precedence
  int nCharsToSkip = 0;
  while (1) {
    // handle '//' comments
    if (!bIgnoreTilCloseComment && c0 == '/' && c1 == '/')
      bIgnoreTilEndOfLine = true;
    if (bIgnoreTilEndOfLine && (c0 == 10 || c0 == 13)) {
      bIgnoreTilEndOfLine = false;
      nCharsToSkip = 0;
    }

    // handle /* */ comments
    if (!bIgnoreTilEndOfLine && c0 == '/' && c1 == '*')
      bIgnoreTilCloseComment = true;
    if (bIgnoreTilCloseComment && c0 == '*' && c1 == '/') {
      bIgnoreTilCloseComment = false;
      nCharsToSkip = 2;
    }

    if (!bIgnoreTilEndOfLine && !bIgnoreTilCloseComment) {
      if (nCharsToSkip > 0)
        nCharsToSkip--;
      else
        *dest++ = c0;
    }

    if (c1 == 0)
      break;

    p++;
    c0 = c1;
    c1 = *p;
  }

  *dest++ = 0;
}

void CancelThread(int max_wait_time_ms) {
  g_bThreadShouldQuit = true;
  int waited = 0;
  while (g_bThreadAlive && waited < max_wait_time_ms) {
    Sleep(30);
    waited += 30;
  }

  if (g_bThreadAlive) {
    TerminateThread(g_hThread, 0);
    g_bThreadAlive = false;
  }

  if (g_hThread != INVALID_HANDLE_VALUE)
    CloseHandle(g_hThread);
  g_hThread = INVALID_HANDLE_VALUE;
}

//----------------------------------------------------------------------

float SquishToCenter(float x, float fExp) {
  if (x > 0.5f)
    return powf(x * 2 - 1, fExp) * 0.5f + 0.5f;

  return (1 - powf(1 - x * 2, fExp)) * 0.5f;
}

int GetNearestPow2Size(int w, int h) {
  float fExp = logf(max(w, h) * 0.75f + 0.25f * min(w, h)) / logf(2.0f);
  float bias = 0.55f;
  if (fExp + bias >= 11.0f)  // ..don't jump to 2048x2048 quite as readily
    bias = 0.5f;
  int nExp = (int)(fExp + bias);
  int log2size = (int)powf(2.0f, (float)nExp);
  return log2size;
}

float fCubicInterpolate(float y0, float y1, float y2, float y3, float t) {
  float a0, a1, a2, a3, t2;

  t2 = t * t;
  a0 = y3 - y2 - y0 + y1;
  a1 = y0 - y1 - a0;
  a2 = y2 - y0;
  a3 = y1;

  return (a0 * t * t2 + a1 * t2 + a2 * t + a3);
}

DWORD dwCubicInterpolate(DWORD y0, DWORD y1, DWORD y2, DWORD y3, float t) {
  // performs cubic interpolation on a D3DCOLOR value.
  DWORD ret = 0;
  DWORD shift = 0;
  for (int i = 0; i < 4; i++) {
    float f = fCubicInterpolate(
        ((y0 >> shift) & 0xFF) / 255.0f,
        ((y1 >> shift) & 0xFF) / 255.0f,
        ((y2 >> shift) & 0xFF) / 255.0f,
        ((y3 >> shift) & 0xFF) / 255.0f,
        t);
    if (f < 0)
      f = 0;
    if (f > 1)
      f = 1;
    ret |= ((DWORD)(f * 255)) << shift;
    shift += 8;
  }
  return ret;
}

//----------------------------------------------------------------------
// Remaining utility and misc methods
//----------------------------------------------------------------------

void CPlugin::SendPresetChangedInfoToMilkwaveRemote() {
  std::wstring msg = L"PRESET=" + std::wstring(m_szCurrentPresetFile);
  SendMessageToMilkwaveRemote(msg.c_str(), true);
  SendPresetWaveInfoToMilkwaveRemote();
}

void CPlugin::SendPresetWaveInfoToMilkwaveRemote() {
  std::wstring msg = L"WAVE|COLORR=" + std::to_wstring(static_cast<int>(std::ceil(g_plugin.m_pState->m_fWaveR.eval(-1) * 255))) + L"|COLORG=" + std::to_wstring(static_cast<int>(std::ceil(g_plugin.m_pState->m_fWaveG.eval(-1) * 255))) + L"|COLORB=" + std::to_wstring(static_cast<int>(std::ceil(g_plugin.m_pState->m_fWaveB.eval(-1) * 255))) + L"|ALPHA=" + std::to_wstring(g_plugin.m_pState->m_fWaveAlpha.eval(-1)) + L"|MODE=" + std::to_wstring(static_cast<int>(g_plugin.m_pState->m_nWaveMode)) + L"|PUSHX=" + std::to_wstring(g_plugin.m_pState->m_fXPush.eval(-1)) + L"|PUSHY=" + std::to_wstring(g_plugin.m_pState->m_fYPush.eval(-1)) + L"|ZOOM=" + std::to_wstring(g_plugin.m_pState->m_fZoom.eval(-1)) + L"|WARP=" + std::to_wstring(g_plugin.m_pState->m_fWarpAmount.eval(-1)) + L"|ROTATION=" + std::to_wstring(g_plugin.m_pState->m_fRot.eval(-1)) + L"|DECAY=" + std::to_wstring(g_plugin.m_pState->m_fDecay.eval(-1)) + L"|SCALE=" + std::to_wstring(g_plugin.m_pState->m_fWaveScale.eval(-1)) + L"|ECHO=" + std::to_wstring(g_plugin.m_pState->m_fVideoEchoZoom.eval(-1)) + L"|BRIGHTEN=" + (g_plugin.m_pState->m_bBrighten ? L"1" : L"0") + L"|DARKEN=" + (g_plugin.m_pState->m_bDarken ? L"1" : L"0") + L"|SOLARIZE=" + (g_plugin.m_pState->m_bSolarize ? L"1" : L"0") + L"|INVERT=" + (g_plugin.m_pState->m_bInvert ? L"1" : L"0") + L"|ADDITIVE=" + (g_plugin.m_pState->m_bAdditiveWaves ? L"1" : L"0") + L"|DOTTED=" + (g_plugin.m_pState->m_bWaveDots ? L"1" : L"0") + L"|THICK=" + (g_plugin.m_pState->m_bWaveThick ? L"1" : L"0") + L"|VOLALPHA=" + (g_plugin.m_pState->m_bModWaveAlphaByVolume ? L"1" : L"0");
  SendMessageToMilkwaveRemote(msg.c_str(), true);
}

void CPlugin::SendSettingsInfoToMilkwaveRemote() {
  // The SETTINGS| message uses '|' as the key/value delimiter, so any '|' in
  // the lyrics status or current line text must be sanitized.
  std::wstring lyricsStatus = ::milkwave.LyricsMonitorText(m_lyricsDisplayEnabled, m_lyricsOffsetMs);
  std::wstring lyricsLine = m_lyricsDisplayEnabled ? ::milkwave.CurrentLyricText(m_lyricsOffsetMs) : L"";
  for (auto& ch : lyricsStatus) {
    if (ch == L'|') ch = L' ';
  }
  for (auto& ch : lyricsLine) {
    if (ch == L'|') ch = L' ';
  }
  m_lastSentLyricsStatus = lyricsStatus;
  m_lastSentLyricsLine = lyricsLine;

  std::wstring msg = L"SETTINGS|ACTIVE=" + std::wstring(bSpoutOut ? L"1" : L"0") + L"|FIXEDSIZE=" + std::wstring(bSpoutFixedSize ? L"1" : L"0") + L"|FIXEDWIDTH=" + std::to_wstring(nSpoutFixedWidth) + L"|FIXEDHEIGHT=" + std::to_wstring(nSpoutFixedHeight) + L"|QUALITY=" + std::to_wstring(m_fRenderQuality) + L"|AUTO=" + std::wstring(bQualityAuto ? L"1" : L"0") + L"|HUE=" + std::to_wstring(m_ColShiftHue) + L"|LOCKED=" + std::wstring(m_bPresetLockedByUser ? L"1" : L"0") + L"|RANDOM=" + std::wstring(m_bSequentialPresetOrder ? L"0" : L"1") + L"|INPUTTOP=" + std::wstring(m_bInputMixOnTop ? L"1" : L"0") + L"|LUMAACTIVE=" + std::wstring(m_bInputMixLumaActive ? L"1" : L"0") + L"|LUMATHR=" + std::to_wstring((int)(m_fInputMixLumakeyThreshold * 100.0f)) + L"|LUMASOFT=" + std::to_wstring((int)(m_fInputMixLumakeySoftness * 100.0f)) + L"|EQATTACK=" + std::to_wstring(m_fEQAttackGlobal) + L"|EQDECAY=" + std::to_wstring(m_fEQDecayGlobal) + L"|EQBOOST=" + std::to_wstring(m_fEQBoostGlobal) + L"|LYRICSACTIVE=" + std::wstring(m_lyricsDisplayEnabled ? L"1" : L"0") + L"|LYRICSAUTO=" + std::wstring(m_bLyricsAutoLoad ? L"1" : L"0") + L"|LYRICSSTATUS=" + lyricsStatus + L"|LYRICSLINE=" + lyricsLine;
  SendMessageToMilkwaveRemote(msg.c_str(), true);
}

void CPlugin::CaptureScreenshot() {
  wchar_t filename[MAX_PATH];
  CaptureScreenshotWithFilename(filename, MAX_PATH);
}

bool CPlugin::CaptureScreenshotWithFilename(wchar_t* outFilename, size_t outFilenameSize) {
  LPDIRECT3DDEVICE9EX pDevice = GetDevice();
  if (!pDevice) {
    OutputDebugStringW(L"[CaptureScreenshot] ERROR: Device not available\n");
    milkwave->LogInfo(L"CaptureScreenshot: Device not available");
    return false;
  }

  IDirect3DSurface9* pRenderTarget = nullptr;
  HRESULT hr = pDevice->GetRenderTarget(0, &pRenderTarget);
  if (FAILED(hr) || !pRenderTarget) {
    wchar_t msg[256];
    swprintf_s(msg, 256, L"CaptureScreenshot: Failed to get render target (HRESULT 0x%08X)", hr);
    milkwave->LogInfo(msg);
    return false;
  }

  wchar_t presetName[MAX_PATH] = L"unknown";
  if (m_szCurrentPresetFile[0]) {
    wchar_t* filenameOnly = wcsrchr(m_szCurrentPresetFile, L'\\');
    if (filenameOnly) {
      filenameOnly++;
    } else {
      filenameOnly = m_szCurrentPresetFile;
    }

    wcsncpy_s(presetName, MAX_PATH, filenameOnly, _TRUNCATE);

    wchar_t* ext = wcsrchr(presetName, L'.');
    if (ext) *ext = L'\0';

    for (wchar_t* p = presetName; *p; p++) {
      if (*p == L'/' || *p == L':' || *p == L'*' ||
          *p == L'?' || *p == L'"' || *p == L'<' || *p == L'>' || *p == L'|') {
        *p = L'_';
      }
    }
  }

  wchar_t captureDir[MAX_PATH];
  swprintf_s(captureDir, MAX_PATH, L"%scapture\\", m_szBaseDir);

  wchar_t debugMsg[MAX_PATH + 50];
  swprintf_s(debugMsg, MAX_PATH + 50, L"[CaptureScreenshot] BaseDir: %s\n", m_szBaseDir);
  OutputDebugStringW(debugMsg);
  swprintf_s(debugMsg, MAX_PATH + 50, L"[CaptureScreenshot] CaptureDir: %s\n", captureDir);
  OutputDebugStringW(debugMsg);

  CreateDirectoryW(captureDir, NULL);

  SYSTEMTIME st;
  GetLocalTime(&st);

  wchar_t justFilename[MAX_PATH];
  swprintf_s(justFilename, MAX_PATH, L"%04d%02d%02d-%02d%02d%02d-%s.png",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond,
             presetName);

  wchar_t fullPath[MAX_PATH];
  swprintf_s(fullPath, MAX_PATH, L"%s%s", captureDir, justFilename);

  hr = D3DXSaveSurfaceToFileW(fullPath, D3DXIFF_PNG, pRenderTarget, NULL, NULL);

  wchar_t resultMsg[MAX_PATH + 100];
  swprintf_s(resultMsg, MAX_PATH + 100, L"[CaptureScreenshot] D3DXSaveSurfaceToFileW result: 0x%08X\n", hr);
  OutputDebugStringW(resultMsg);

  pRenderTarget->Release();

  if (SUCCEEDED(hr)) {
    wchar_t msg[512];
    swprintf_s(msg, 512, L"Screenshot saved: %s", fullPath);
    milkwave->LogInfo(msg);

    wchar_t successMsg[MAX_PATH + 50];
    swprintf_s(successMsg, MAX_PATH + 50, L"[CaptureScreenshot] SUCCESS: %s\n", justFilename);
    OutputDebugStringW(successMsg);

    if (outFilename && outFilenameSize > 0) {
      wcsncpy_s(outFilename, outFilenameSize, justFilename, _TRUNCATE);
    }
    return true;
  } else {
    wchar_t msg[256];
    swprintf_s(msg, 256, L"Failed to save screenshot: HRESULT 0x%08X", hr);
    milkwave->LogInfo(msg);

    wchar_t errorMsg[100];
    swprintf_s(errorMsg, 100, L"[CaptureScreenshot] FAILED: HRESULT 0x%08X\n", hr);
    OutputDebugStringW(errorMsg);

    return false;
  }
}

void CPlugin::SetWaveParamsFromMessage(std::wstring& message) {
  std::wstringstream ss(message);
  std::wstring token;
  std::map<std::wstring, std::wstring> params;

  // Parse key-value pairs
  while (std::getline(ss, token, L'|')) {
    size_t pos = token.find(L'=');
    if (pos != std::wstring::npos) {
      std::wstring key = token.substr(0, pos);
      std::wstring value = token.substr(pos + 1);
      params[key] = value;
    }
  }

  if (params.find(L"MODE") != params.end()) {
    g_plugin.m_pState->m_nWaveMode = std::stoi(params[L"MODE"]);
  }
  if (params.find(L"ALPHA") != params.end()) {
    g_plugin.m_pState->m_fWaveAlpha = std::stof(params[L"ALPHA"]);
  }
  if (params.find(L"COLORR") != params.end()) {
    int colR = std::stoi(params[L"COLORR"]);
    float colRDbl = colR / 255.0f;
    g_plugin.m_pState->m_fWaveR = colRDbl;
    g_plugin.m_pState->m_fMvR = colRDbl;
  }
  if (params.find(L"COLORG") != params.end()) {
    int colG = std::stoi(params[L"COLORG"]);
    float colGDbl = colG / 255.0f;
    g_plugin.m_pState->m_fWaveG = colGDbl;
    g_plugin.m_pState->m_fMvG = colGDbl;
  }
  if (params.find(L"COLORB") != params.end()) {
    int colB = std::stoi(params[L"COLORB"]);
    float colBDbl = colB / 255.0f;
    g_plugin.m_pState->m_fWaveB = colBDbl;
    g_plugin.m_pState->m_fMvB = colBDbl;
  }
  if (params.find(L"PUSHX") != params.end()) {
    g_plugin.m_pState->m_fXPush = std::stof(params[L"PUSHX"]);
  }
  if (params.find(L"PUSHY") != params.end()) {
    g_plugin.m_pState->m_fYPush = std::stof(params[L"PUSHY"]);
  }
  if (params.find(L"ZOOM") != params.end()) {
    g_plugin.m_pState->m_fZoom = std::stof(params[L"ZOOM"]);
  }
  if (params.find(L"WARP") != params.end()) {
    g_plugin.m_pState->m_fWarpAmount = std::stof(params[L"WARP"]);
  }
  if (params.find(L"ROTATION") != params.end()) {
    g_plugin.m_pState->m_fRot = std::stof(params[L"ROTATION"]);
  }
  if (params.find(L"DECAY") != params.end()) {
    g_plugin.m_pState->m_fDecay = std::stof(params[L"DECAY"]);
  }
  if (params.find(L"SCALE") != params.end()) {
    g_plugin.m_pState->m_fWaveScale = std::stof(params[L"SCALE"]);
  }
  if (params.find(L"ECHO") != params.end()) {
    g_plugin.m_pState->m_fVideoEchoZoom = std::stof(params[L"ECHO"]);
  }
  if (params.find(L"BRIGHTEN") != params.end()) {
    g_plugin.m_pState->m_bBrighten = params[L"BRIGHTEN"] == L"1";
  }
  if (params.find(L"DARKEN") != params.end()) {
    g_plugin.m_pState->m_bDarken = params[L"DARKEN"] == L"1";
  }
  if (params.find(L"SOLARIZE") != params.end()) {
    g_plugin.m_pState->m_bSolarize = params[L"SOLARIZE"] == L"1";
  }
  if (params.find(L"INVERT") != params.end()) {
    g_plugin.m_pState->m_bInvert = params[L"INVERT"] == L"1";
  }
  if (params.find(L"ADDITIVE") != params.end()) {
    g_plugin.m_pState->m_bAdditiveWaves = params[L"ADDITIVE"] == L"1";
  }
  if (params.find(L"DOTTED") != params.end()) {
    g_plugin.m_pState->m_bWaveDots = params[L"DOTTED"] == L"1";
  }
  if (params.find(L"THICK") != params.end()) {
    g_plugin.m_pState->m_bWaveThick = params[L"THICK"] == L"1";
  }
  if (params.find(L"VOLALPHA") != params.end()) {
    g_plugin.m_pState->m_bModWaveAlphaByVolume = params[L"VOLALPHA"] == L"1";
  }
}

bool CPlugin::LaunchSprite(int nSpriteNum, int nSlot) {
  char initcode[8192], code[8192], sectionA[64];
  char szTemp[8192];
  wchar_t img[512], section[64];

  initcode[0] = 0;
  code[0] = 0;
  img[0] = 0;
  swprintf(section, L"img%02d", nSpriteNum);
  sprintf(sectionA, "img%02d", nSpriteNum);

  // 1. read in image filename
  GetPrivateProfileStringW(section, L"img", L"", img, sizeof(img) - 1, m_szImgIniFile);
  if (img[0] == 0) {
    wchar_t buf[1024];
    swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_ERROR_COULD_NOT_FIND_IMG_OR_NOT_DEFINED), nSpriteNum);
    AddError(buf, 7.0f, ERR_MISC, false);
    return false;
  }

  if (img[1] != L':')  // || img[2] != '\\')
  {
    // it's not in the form "x:\blah\billy.jpg" so prepend plugin dir path.
    wchar_t temp[512];
    wcscpy(temp, img);
    swprintf(img, L"%s%s", m_szMilkdrop2Path, temp);
  }

  // 2. get color key
  // unsigned int ck_lo = (unsigned int)GetPrivateProfileInt(section, "colorkey_lo", 0x00000000, m_szImgIniFile);
  // unsigned int ck_hi = (unsigned int)GetPrivateProfileInt(section, "colorkey_hi", 0x00202020, m_szImgIniFile);
  // FIRST try 'colorkey_lo' (for backwards compatibility) and then try 'colorkey'
  unsigned int ck = (unsigned int)GetPrivateProfileIntW(section, L"colorkey_lo", 0x00000000, m_szImgIniFile /*GetConfigIniFile()*/);
  ck = (unsigned int)GetPrivateProfileIntW(section, L"colorkey", ck, m_szImgIniFile /*GetConfigIniFile()*/);

  // 3. read in init code & per-frame code
  for (int n = 0; n < 2; n++) {
    char* pStr = (n == 0) ? initcode : code;
    char szLineName[32];
    int len;

    int line = 1;
    int char_pos = 0;
    bool bDone = false;

    while (!bDone) {
      if (n == 0)
        sprintf(szLineName, "init_%d", line);
      else
        sprintf(szLineName, "code_%d", line);

      GetPrivateProfileString(sectionA, szLineName, "~!@#$", szTemp, 8192, AutoCharFn(m_szImgIniFile));  // fixme
      len = lstrlen(szTemp);

      if ((strcmp(szTemp, "~!@#$") == 0) ||  // if the key was missing,
          (len >= 8191 - char_pos - 1))      // or if we're out of space
      {
        bDone = true;
      } else {
        sprintf(&pStr[char_pos], "%s%c", szTemp, LINEFEED_CONTROL_CHAR);
      }

      char_pos += len + 1;
      line++;
    }
    pStr[char_pos++] = 0;  // null-terminate
  }

  if (nSlot == -1) {
    // find first empty slot; if none, chuck the oldest sprite & take its slot.
    int oldest_index = 0;
    int oldest_frame = m_texmgr.m_tex[0].nStartFrame;
    for (int x = 0; x < NUM_TEX; x++) {
      if (!m_texmgr.m_tex[x].pSurface) {
        nSlot = x;
        break;
      } else if (m_texmgr.m_tex[x].nStartFrame < oldest_frame) {
        oldest_index = x;
        oldest_frame = m_texmgr.m_tex[x].nStartFrame;
      }
    }

    if (nSlot == -1) {
      nSlot = oldest_index;
      m_texmgr.KillTex(nSlot);
    }
  }

  int ret = m_texmgr.LoadTex(img, nSlot, initcode, code, GetTime(), GetFrame(), ck);
  m_texmgr.m_tex[nSlot].nUserData = nSpriteNum;

  wchar_t buf[1024];
  switch (ret & TEXMGR_ERROR_MASK) {
    case TEXMGR_ERR_SUCCESS:
      switch (ret & TEXMGR_WARNING_MASK) {
        case TEXMGR_WARN_ERROR_IN_INIT_CODE:
          swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_WARNING_ERROR_IN_INIT_CODE), nSpriteNum);
          AddError(buf, 6.0f, ERR_MISC, true);
          break;
        case TEXMGR_WARN_ERROR_IN_REG_CODE:
          swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_WARNING_ERROR_IN_PER_FRAME_CODE), nSpriteNum);
          AddError(buf, 6.0f, ERR_MISC, true);
          break;
        default:
          // success; no errors OR warnings.
          break;
      }
      break;
    case TEXMGR_ERR_BAD_INDEX:
      swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_ERROR_BAD_SLOT_INDEX), nSpriteNum);
      AddError(buf, 6.0f, ERR_MISC, true);
      break;
      /*
        case TEXMGR_ERR_OPENING:                sprintf(m_szUserMessage, "sprite #%d error: unable to open imagefile", nSpriteNum); break;
      case TEXMGR_ERR_FORMAT:                 sprintf(m_szUserMessage, "sprite #%d error: file is corrupt or non-jpeg image", nSpriteNum); break;
      case TEXMGR_ERR_IMAGE_NOT_24_BIT:       sprintf(m_szUserMessage, "sprite #%d error: image does not have 3 color channels", nSpriteNum); break;
      case TEXMGR_ERR_IMAGE_TOO_LARGE:        sprintf(m_szUserMessage, "sprite #%d error: image is too large", nSpriteNum); break;
      case TEXMGR_ERR_CREATESURFACE_FAILED:   sprintf(m_szUserMessage, "sprite #%d error: createsurface() failed", nSpriteNum); break;
      case TEXMGR_ERR_LOCKSURFACE_FAILED:     sprintf(m_szUserMessage, "sprite #%d error: lock() failed", nSpriteNum); break;
      case TEXMGR_ERR_CORRUPT_JPEG:           sprintf(m_szUserMessage, "sprite #%d error: jpeg is corrupt", nSpriteNum); break;
        */
    case TEXMGR_ERR_BADFILE:
      swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_ERROR_IMAGE_FILE_MISSING_OR_CORRUPT), nSpriteNum);
      AddError(buf, 6.0f, ERR_MISC, true);
      break;
    case TEXMGR_ERR_OUTOFMEM:
      swprintf(buf, wasabiApiLangString(IDS_SPRITE_X_ERROR_OUT_OF_MEM), nSpriteNum);
      AddError(buf, 6.0f, ERR_MISC, true);
      break;
  }

  return (ret & TEXMGR_ERROR_MASK) ? false : true;
}

void CPlugin::KillSprite(int iSlot) {
  m_texmgr.KillTex(iSlot);
}

void CPlugin::KillMilk2Sprites() {
  for (int i = 0; i < MAX_MILK2_SPRITES; i++) {
    if (m_nMilk2SpriteSlots[i] >= 0) {
      m_texmgr.KillTex(m_nMilk2SpriteSlots[i]);
      m_nMilk2SpriteSlots[i] = -1;
    }
  }
}

void CPlugin::LaunchMilk2Sprites() {
  KillMilk2Sprites();

  for (int si = 0; si < m_nMilk2SpriteCount; si++) {
    Milk2Sprite& spr = m_milk2Sprites[si];
    if (!spr.valid) continue;

    // Find a free texmgr slot (or evict the oldest).
    int nSlot = -1;
    int oldest_index = 0;
    int oldest_frame = m_texmgr.m_tex[0].nStartFrame;
    for (int x = 0; x < NUM_TEX; x++) {
      if (!m_texmgr.m_tex[x].pSurface) {
        nSlot = x;
        break;
      } else if (m_texmgr.m_tex[x].nStartFrame < oldest_frame) {
        oldest_index = x;
        oldest_frame = m_texmgr.m_tex[x].nStartFrame;
      }
    }
    if (nSlot == -1) {
      nSlot = oldest_index;
      m_texmgr.KillTex(nSlot);
    }

    // Build init code: set initial property values from the milk2 sprite header.
    // Map MilkDrop 3 blendmode values to the 0-4 range supported by the texmgr renderer:
    //   MD3 0-4 = same as MD2
    //   MD3 5   = additive with alpha (map to 2 additive)
    //   MD3 7   = decal with texture alpha (map to 4 colorkey)
    //   MD3 9   = srccolor additive (map to 3 srccolor)
    //   MD3 10  = multiply (map to 3 srccolor, closest match)
    //   others  = map to 4 colorkey (safe default with alpha)
    int mappedBlend = spr.nBlend;
    if (mappedBlend > 4) {
      switch (spr.nBlend) {
        case 5:
          mappedBlend = 2;
          break;  // additive
        case 7:
          mappedBlend = 4;
          break;  // colorkey
        case 9:
          mappedBlend = 3;
          break;  // srccolor
        case 10:
          mappedBlend = 3;
          break;  // srccolor
        default:
          mappedBlend = 4;
          break;  // colorkey as safe default
      }
    }

    // Convert SpriteX/SpriteY to texmgr's 0..1 coordinate space:
    // MilkDrop 3 uses 0,0 = center; map to texmgr's x=0.5,y=0.5
    float texX = spr.fX + 0.5f;
    float texY = spr.fY + 0.5f;

    // In MilkDrop 3, SpriteSX/SpriteSY act as persistent scale multipliers
    // applied AFTER per-frame code, not as initial sx/sy values (which per-frame
    // code would overwrite). Store them as _bsx/_bsy and append multiplication
    // at the end of the per-frame code.
    //
    // Pre-divide by screen aspect so SpriteSX/SY are height-relative (MD3 semantics):
    // the 2nd AR correction (y *= aspect) in DrawUserSprites then restores visual
    // circularity, and the sprite stays within screen bounds on any aspect ratio.
    float screenAspect = (GetHeight() > 0) ? (GetWidth() / (float)GetHeight()) : 1.0f;  // default to 0.8 if height is zero for some reason
    float adjustedSX = spr.fSX / screenAspect * 1.2f;
    float adjustedSY = spr.fSY / screenAspect * 1.2f;

    char initcode[8192];
    snprintf(initcode, sizeof(initcode),
             "x=%f;%c"
             "y=%f;%c"
             "sx=1;%c"
             "sy=1;%c"
             "rot=0;%c"
             "a=%f;%c"
             "blendmode=%d;%c"
             "burn=%d;%c"
             "repeatx=%f;%c"
             "repeaty=%f;%c"
             "done=0;%c"
             "_bsx=%f;%c"
             "_bsy=%f;%c",
             texX, LINEFEED_CONTROL_CHAR,
             texY, LINEFEED_CONTROL_CHAR,
             LINEFEED_CONTROL_CHAR,
             LINEFEED_CONTROL_CHAR,
             LINEFEED_CONTROL_CHAR,
             spr.fAlpha, LINEFEED_CONTROL_CHAR,
             mappedBlend, LINEFEED_CONTROL_CHAR,
             spr.bBurn ? 1 : 0, LINEFEED_CONTROL_CHAR,
             spr.fRepeatX, LINEFEED_CONTROL_CHAR,
             spr.fRepeatY, LINEFEED_CONTROL_CHAR,
             LINEFEED_CONTROL_CHAR,
             adjustedSX, LINEFEED_CONTROL_CHAR,
             adjustedSY, LINEFEED_CONTROL_CHAR);

    // Append user init code from the milk2 file
    if (spr.szInitCode[0]) {
      size_t len = strlen(initcode);
      strncat_s(initcode, sizeof(initcode), spr.szInitCode, sizeof(initcode) - len - 1);
    }

    // Build per-frame code: prepend rotation animation, then user code.
    char code[8192];
    code[0] = 0;
    // SpriteSpeed is in radians/second; SpriteRot is a direction multiplier (+1/-1).
    if (spr.fRot != 0.0f && spr.fSpeed != 0.0f) {
      snprintf(code, sizeof(code), "rot=rot+%f/fps;%c",
               spr.fSpeed * spr.fRot, LINEFEED_CONTROL_CHAR);
    }
    // Append user per-frame code from the milk2 file
    if (spr.szCode[0]) {
      size_t len = strlen(code);
      strncat_s(code, sizeof(code), spr.szCode, sizeof(code) - len - 1);
    }

    // Append base scale multiplication: apply SpriteSX/SpriteSY as persistent
    // multipliers AFTER all per-frame code, so user code like "sx=new_scale"
    // gets scaled by the base sprite dimensions rather than overwriting them.
    {
      char scaleCode[128];
      snprintf(scaleCode, sizeof(scaleCode), "sx=sx*_bsx;%csy=sy*_bsy;%c",
               LINEFEED_CONTROL_CHAR, LINEFEED_CONTROL_CHAR);
      size_t len = strlen(code);
      strncat_s(code, sizeof(code), scaleCode, sizeof(code) - len - 1);
    }

    int ret = m_texmgr.LoadTex(spr.szImgPath, nSlot, initcode, code, GetTime(), GetFrame(), spr.nColorKey);
    if ((ret & TEXMGR_ERROR_MASK) == TEXMGR_ERR_SUCCESS) {
      m_nMilk2SpriteSlots[si] = nSlot;
      m_texmgr.m_tex[nSlot].nUserData = -1;  // mark as milk2 sprite (not a user-launched sprite)
    } else {
      wchar_t buf[1024];
      swprintf(buf, L"milk2 sprite: failed to load '%s' (error %d)", spr.szImgPath, ret);
      AddError(buf, 6.0f, ERR_MISC, true);
      m_nMilk2SpriteSlots[si] = -1;
    }
  }
}

int SAMPLE_RATE = 44100;  // Initialize sample rate globally, 44100hz is the default sample rate for MilkDrop

HRESULT DetectSampleRate() {
  HRESULT hr = S_OK;
  IMMDeviceEnumerator* pEnumerator = NULL;
  IMMDevice* pDevice = NULL;
  IPropertyStore* pProps = NULL;
  PROPVARIANT var;
  PropVariantInit(&var);

  // Initialize COM
  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (FAILED(hr)) return hr;

  // Create device enumerator
  hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                        (void**)&pEnumerator);
  if (FAILED(hr)) goto Cleanup;

  // Get default audio endpoint
  hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
  if (FAILED(hr)) goto Cleanup;

  // Open property store
  hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
  if (FAILED(hr)) goto Cleanup;

  // Get the format property
  hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &var);
  if (SUCCEEDED(hr) && var.vt == VT_BLOB) {
    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)var.blob.pBlobData;
    if (pwfx != NULL) {
      SAMPLE_RATE = pwfx->nSamplesPerSec;
    }
  }

Cleanup:
  // Clean up
  PropVariantClear(&var);
  if (pProps) pProps->Release();
  if (pDevice) pDevice->Release();
  if (pEnumerator) pEnumerator->Release();
  CoUninitialize();

  return hr;
}

int CPlugin::GetNextFreeSupertextIndex() {
  int index = 0;
  for (int i = 0; i < NUM_SUPERTEXTS; i++) {
    if (m_supertexts[i].fStartTime == -1.0f) {
      index = i;
      break;
    }
  }
  // if no text is free, we'll reset and use index=0
  m_supertexts[index] = td_supertext();  // Reset the supertext at this index
  return index;
}

void CPlugin::DoCustomSoundAnalysis() {
  // Now uses configurations via beatdrop.ini, don't modify here.
  // Bass
  int BASS_MIN = m_nBassStart;
  int BASS_MAX = m_nBassEnd;

  // Middle
  int MID_MIN = m_nMidStart;
  int MID_MAX = m_nMidEnd;

  // Treble
  int TREBLE_MIN = m_nTrebStart;
  int TREBLE_MAX = m_nTrebEnd;

  // This uses the sample rate dependent on your speaker device.
  // Beat Detection Configuration
  // Look at the start of line 10566 for the new beat detection splitting algorithm.

  memcpy(mysound.fWave[0], m_sound.fWaveform[0], sizeof(float) * 576);
  memcpy(mysound.fWave[1], m_sound.fWaveform[1], sizeof(float) * 576);

  // Read audio: large buffer for shader FFT, last 576 samples for legacy FFT
  float fWaveLargeL[MAX_FFT_SHADER_INPUT];
  float fWaveLargeR[MAX_FFT_SHADER_INPUT];
  int nReadSamples = max(m_nFFTShaderInput, 576);  // always enough for legacy 576-sample FFT
  GetAudioBufFloat(fWaveLargeL, fWaveLargeR, nReadSamples);
  float* fWaveLeft = fWaveLargeL + (nReadSamples - 576);
  float* fWaveRight = fWaveLargeR + (nReadSamples - 576);
  float* fShaderLeft = fWaveLargeL + (nReadSamples - m_nFFTShaderInput);
  float* fShaderRight = fWaveLargeR + (nReadSamples - m_nFFTShaderInput);

  // Legacy un-normalized FFT (576 samples, 512 bins)
  memset(mysound.fSpecLeft, 0, sizeof(float) * MY_FFT_SAMPLES);
  memset(mysound.fSpecRight, 0, sizeof(float) * MY_FFT_SAMPLES);

  myfft.time_to_frequency_domain(fWaveLeft, mysound.fSpecLeft);
  myfft.time_to_frequency_domain(fWaveRight, mysound.fSpecRight);

  // Compute high-resolution FFT for get_fft()/get_fft_hz() shader functions
  // Window size is configurable via settings.ini FFTSize (default 2048).
  float fShaderSpecLeft[MAX_FFT_SHADER_BINS];
  float fShaderSpecRight[MAX_FFT_SHADER_BINS];
  memset(fShaderSpecLeft, 0, sizeof(float) * m_nFFTShaderBins);
  memset(fShaderSpecRight, 0, sizeof(float) * m_nFFTShaderBins);
  m_fftShader.time_to_frequency_domain(fShaderLeft, fShaderSpecLeft);
  m_fftShader.time_to_frequency_domain(fShaderRight, fShaderSpecRight);

  // Apply FFT smoothing and upload to GPU texture
  {
    float attack = m_pState ? m_pState->m_fFFTAttack : m_fEQAttackGlobal;
    float decay = m_pState ? m_pState->m_fFFTDecay : m_fEQDecayGlobal;
    const float kScaleFactor = m_fEQBoostGlobal * m_fEQBoostGlobal * 0.00030f * ((float)m_nFFTShaderBins / (float)m_nFFTShaderInput);
    const float kNoiseGate = 5e-5f;
    const float kVisibleFloor = 2.5e-4f;
    // Pink noise compensation reference bin at 1 kHz.
    // Applying sqrt(fi/refBin) gives +3 dB/octave slope so that a 1/f spectrum
    // (pink noise, typical of music) displays as a flat EQ response.
    const float kPinkRefBin = 1000.0f * (float)m_nFFTShaderBins / 22050.0f;
    for (int fi = 0; fi < m_nFFTShaderBins; fi++) {
      float mono = (fShaderSpecLeft[fi] + fShaderSpecRight[fi]) * 0.5f;
      // Store linear magnitude in texture; sqrt() is applied in the shader.
      mono = mono * kScaleFactor;
      // Noise gate: zero out values below threshold to suppress window sidelobes.
      if (mono < kNoiseGate) mono = 0.0f;
      // Pink noise compensation: +3 dB/octave, normalized to 1.0 at 1 kHz.
      // Equalizes the natural 1/f amplitude rolloff of music so all EQ bands
      // display at comparable heights for a spectrally balanced signal.
      mono *= sqrtf((float)(fi + 1) / kPinkRefBin);
      // High-frequency bins have shorter periods per wavelength so frame-to-frame
      // variance is higher. Blend towards more temporal smoothing above ~4 kHz
      // so high bands appear as stable as low ones when playing a steady tone.
      // Below 4 kHz: use preset attack; above 16 kHz: use ~50% of attack.
      const float kHiFreqBinLo = 4000.0f * (float)m_nFFTShaderBins / 22050.0f;
      const float kHiFreqBinHi = 16000.0f * (float)m_nFFTShaderBins / 22050.0f;
      float hiBlend = clamp((float)(fi - kHiFreqBinLo) / (kHiFreqBinHi - kHiFreqBinLo), 0.0f, 1.0f);
      float eff_attack = attack * (0.7f - 0.2f * hiBlend);
      if (mono > m_fFFTSmoothed[fi])
        m_fFFTSmoothed[fi] += (mono - m_fFFTSmoothed[fi]) * eff_attack;
      else {
        float decayFactor = (1.0f - decay) * (1.0f - decay) * 0.75f;
        m_fFFTSmoothed[fi] += (mono - m_fFFTSmoothed[fi]) * decayFactor;
      }
      if (m_fFFTSmoothed[fi] < kVisibleFloor)
        m_fFFTSmoothed[fi] = 0.0f;
    }
    // Update peak hold: hold for ~0.5 seconds then decay
    for (int fi = 0; fi < m_nFFTShaderBins; fi++) {
      if (m_fFFTSmoothed[fi] >= m_fFFTPeak[fi]) {
        m_fFFTPeak[fi] = m_fFFTSmoothed[fi];
        m_nFFTPeakHold[fi] = 30;
      } else if (m_nFFTPeakHold[fi] > 0) {
        m_nFFTPeakHold[fi]--;
      } else {
        m_fFFTPeak[fi] *= 0.97f;
        if (m_fFFTPeak[fi] < kVisibleFloor) m_fFFTPeak[fi] = 0.0f;
      }
    }
    if (m_lpFFTTexture) {
      D3DLOCKED_RECT r;
      if (D3D_OK == m_lpFFTTexture->LockRect(0, &r, NULL, D3DLOCK_DISCARD)) {
        float* row0 = (float*)r.pBits;
        float* row1 = (float*)((BYTE*)r.pBits + r.Pitch);
        for (int fi = 0; fi < m_nFFTShaderBins; fi++) {
          row0[fi] = m_fFFTSmoothed[fi];
          row1[fi] = m_fFFTPeak[fi];
        }
        m_lpFFTTexture->UnlockRect(0);
      }
    }
  }

  // DeepSeek - Update the sample rate (we don't need to check HRESULT every frame)
  static DWORD lastCheck = 0;
  DWORD currentTime = GetTickCount();
  if (currentTime - lastCheck > 5000)  // Check once per second
  {
    DetectSampleRate();
    lastCheck = currentTime;
  }

  // sum spectrum up into 3 bands
  // DeepSeek - Updated Beat Detection Splitting Algorithm
  int i;
  for (i = 0; i < 3; i++) {
    // Calculate which FFT bins correspond to our frequency ranges
    int start_bin, end_bin;

    switch (i) {
      case 0:  // Bass (0-250Hz)
        start_bin = (int)(BASS_MIN * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        end_bin = (int)(BASS_MAX * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        break;
      case 1:  // Mid (250-4000Hz)
        start_bin = (int)(MID_MIN * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        end_bin = (int)(MID_MAX * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        break;
      case 2:  // Treble (4000-20000Hz)
        start_bin = (int)(TREBLE_MIN * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        end_bin = (int)(TREBLE_MAX * MY_FFT_SAMPLES / (SAMPLE_RATE / 2));
        break;
    }

    // Clamp values to valid range
    start_bin = max(0, min(start_bin, MY_FFT_SAMPLES - 1));
    end_bin = max(0, min(end_bin, MY_FFT_SAMPLES - 1));

    mysound.imm[i] = 0;  // To prevent the waveform's spikyness and performance lag

    // Sum the energy in the frequency range
    for (int j = start_bin; j <= end_bin; j++) {
      mysound.imm[i] += (mysound.fSpecLeft[j] + mysound.fSpecRight[j]);
    }
  }

  int recentBufferSize = static_cast<int>(GetFps());

  // do temporal blending to create attenuated and super-attenuated versions
  for (i = 0; i < 3; i++) {
    float rate;

    if (mysound.imm[i] > mysound.avg[i])
      rate = 0.2f;
    else
      rate = 0.5f;
    rate = AdjustRateToFPS(rate, 30.0f, GetFps());
    mysound.avg[i] = mysound.avg[i] * rate + mysound.imm[i] * (1 - rate);

    if (GetFrame() < 50)
      rate = 0.9f;
    else
      rate = 0.992f;
    rate = AdjustRateToFPS(rate, 30.0f, GetFps());
    mysound.long_avg[i] = mysound.long_avg[i] * rate + mysound.imm[i] * (1 - rate);

    // also get bass/mid/treble levels *relative to the past*
    // changed all the values to 0 instead of 1 when it's no music
    if (fabsf(mysound.long_avg[i]) < 0.001f)
      mysound.imm_rel[i] = 0.0f;
    else
      mysound.imm_rel[i] = mysound.imm[i] / mysound.long_avg[i];

    if (fabsf(mysound.long_avg[i]) < 0.001f)
      mysound.avg_rel[i] = 0.0f;
    else
      mysound.avg_rel[i] = mysound.avg[i] / mysound.long_avg[i];

    if (mysound.recent[i].size() == 0) {
      mysound.recent[i] = std::vector<float>();
    }

    // smooth
    mysound.recent[i].push_back(mysound.imm_rel[i]);
    if (mysound.recent[i].size() > static_cast<size_t>(recentBufferSize)) {
      mysound.recent[i].erase(mysound.recent[i].begin());
    }
    mysound.smooth[i] = 0;
    size_t k = 0;
    for (; k < mysound.recent[i].size(); k++) {
      mysound.smooth[i] += mysound.recent[i][k];
    }
    if (k > 0) {
      mysound.smooth[i] /= k;
    }

    if (fabsf(mysound.long_avg[i]) < 0.001f)
      mysound.smooth_rel[i] = 0.0f;
    else
      mysound.smooth_rel[i] = mysound.smooth[i] / mysound.long_avg[i];

    // wchar_t buffer[256];
    // swprintf(buffer, 256, L"[%i] %5.2f %5.2f %5.2f %5.2f\n", i, mysound.imm[i], mysound.imm_rel[i], mysound.avg_rel[i], mysound.smooth[i]);
    // OutputDebugStringW(buffer);
  }
}

void CPlugin::GetSongTitle(wchar_t* szSongTitle, int nSize) {
  // if (playbackService &&
  //     playbackService->GetPlaybackState() == musik::core::sdk::PlaybackStopped)
  //{
  //     emulatedWinampSongTitle = "Playback Stopped";
  // }
  emulatedWinampSongTitle = "";
  lstrcpynW(szSongTitle, AutoWide(emulatedWinampSongTitle.c_str(), CP_UTF8), nSize);
}

// =========================================================
// SPOUT initialization function
// Initializes OpenGL and a Spout sender
//
bool CPlugin::OpenSender(unsigned int width, unsigned int height) {
  SpoutLogNotice("CPlugin::OpenSender(%d, %d)", width, height);

  // Close existing sender
  if (bInitialized) spoutsender.ReleaseDX9sender();
  bInitialized = false;

  // SPOUT - DX9EX
  // Set up for using the application DX9ex device.
  // The sender shared texture is then created using this device.
  // Only possible for DX9 mode.
  spoutsender.SetDX9device(GetDevice());

  // Give the sender a name
  spoutsender.SetSenderName(WinampSenderName);

  g_Width = width;
  g_Height = height;
  bSpoutOut = true;
  bInitialized = true;

  return true;

}  // end OpenSender

void CPlugin::OpenMilkwaveRemote() {
  HWND hwnd = FindWindowW(NULL, L"Milkwave Remote");
  if (hwnd) {
    // Bring the window to the front
    SetForegroundWindow(hwnd);
    ShowWindow(hwnd, SW_RESTORE);
  } else {
    // Start the program "MilkwaveRemote.exe"
    // Ensure STARTUPINFOW is used for CreateProcessW
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(L"MilkwaveRemote.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
      g_plugin.AddError(L"Could not start Milkwave Remote", 3.0f, ERR_MISC, false);
    } else {
      g_plugin.AddNotification(L"Starting Milkwave Remote");
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
  }
}

void CPlugin::SetAudioDeviceDisplayName(const wchar_t* displayName, bool isRenderDevice) {
  m_nAudioDeviceActiveType = isRenderDevice ? 2 : 1;

  if (displayName == nullptr) {
    m_szAudioDeviceDisplayName[0] = L'\0';
    return;
  }

  std::wstring sanitized(displayName);

  auto removeDuplicateTag = [&sanitized](const wchar_t* tag) {
    size_t first = sanitized.find(tag);
    if (first == std::wstring::npos) {
      return;
    }

    size_t searchPos = first + wcslen(tag);
    while (true) {
      size_t next = sanitized.find(tag, searchPos);
      if (next == std::wstring::npos) {
        break;
      }
      sanitized.erase(next, wcslen(tag));
      if (next > 0 && sanitized[next - 1] == L' ') {
        sanitized.erase(next - 1, 1);
      }
      searchPos = next;
    }
  };

  removeDuplicateTag(L" [In]");
  removeDuplicateTag(L" [Out]");

  // collapse duplicate spaces
  size_t dupSpace;
  while ((dupSpace = sanitized.find(L"  ")) != std::wstring::npos) {
    sanitized.erase(dupSpace, 1);
  }

  wcsncpy_s(m_szAudioDeviceDisplayName, MAX_PATH, sanitized.c_str(), _TRUNCATE);
}

void CPlugin::SetAMDFlag() {
  if (m_AMDDetectionMode == 0) {
    m_IsAMD = is_amd_ati();
  } else if (m_AMDDetectionMode == 1) {
    m_IsAMD = true;
  } else {
    m_IsAMD = false;
  }
}

#include <fstream>

bool CPlugin::CheckDX9DLL() {
  // Try to load the DLL manually
  HMODULE hD3DX = LoadLibrary(TEXT("D3DX9_43.dll"));

  if (!hD3DX) {
    ShowDirectXMissingMessage();
    return false;
  }

  // If successful, free the DLL (optional if you're linking statically)
  FreeLibrary(hD3DX);

  return true;
}

// Test for DirectX installation and warn if not installed
//
// Registry method only works for DirectX 9 and lower but that is OK
bool CPlugin::CheckForDirectX9c() {
  // HKLM\Software\Microsoft\DirectX\Version should be 4.09.00.0904
  // handy information : http://en.wikipedia.org/wiki/DirectX
  HKEY hRegKey;
  LONG regres;
  DWORD dwSize, major, minor, revision, notused;
  char value[256];
  dwSize = 256;

  // Does the key exist
  regres = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DirectX", NULL, KEY_READ, &hRegKey);
  if (regres == ERROR_SUCCESS) {
    // Read the key
    regres = RegQueryValueExA(hRegKey, "Version", 0, NULL, (LPBYTE)value, &dwSize);
    // Decode the string : 4.09.00.0904
    sscanf_s(value, "%d.%d.%d.%d", &major, &minor, &notused, &revision);
    // printf("DirectX registry : [%s] (%d.%d.%d.%d)\n", value, major, minor, notused, revision);
    RegCloseKey(hRegKey);
    if (major == 4 && minor == 9 && revision == 904)
      return true;
  }
  // If we get here, DirectX 9c is not installed
  ShowDirectXMissingMessage();
  return false;
}

void CPlugin::ShowDirectXMissingMessage() {
  if (MessageBoxA(NULL,
                  "Could not initialize DirectX 9.\n\nPlease install the DirectX End-User Legacy Runtimes.\n\nOpen Download-Website now?",
                  "Milkwave Visualizer", MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST) == IDYES) {
    // open website in browser
    ShellExecuteA(NULL, "open", "https://www.microsoft.com/en-us/download/details.aspx?id=35", NULL, NULL, SW_SHOWNORMAL);
  }
}

void CPlugin::CompileInputMixShader() {
  const char* szShader =
      "sampler2D inputTex : register(s0);\n"
      "float4 lumaParams1 : register(c0);\n"  // x: threshold, y: softness, z: opacity (extra), w: active (1.0/0.0)
      "struct PS_INPUT {\n"
      "    float2 uv    : TEXCOORD0;\n"
      "    float4 color : COLOR0;\n"
      "};\n"
      "float4 main(PS_INPUT input) : COLOR {\n"
      "    float4 col = tex2D(inputTex, input.uv);\n"
      "    col *= input.color;\n"
      "    if (lumaParams1.w > 0.5) {\n"
      "        float luma = dot(col.rgb, float3(0.299, 0.587, 0.114));\n"
      "        float factor = saturate((luma - lumaParams1.x) / max(0.0001, lumaParams1.y));\n"
      "        col.a *= factor;\n"
      "    }\n"
      "    return col;\n"
      "}\n";

  ID3DXBuffer* pShaderByteCode = NULL;
  ID3DXBuffer* pErrors = NULL;

  if (SUCCEEDED(D3DXCompileShader(szShader, (UINT)strlen(szShader), NULL, NULL, "main", "ps_2_0", 0, &pShaderByteCode, &pErrors, NULL))) {
    if (m_lpPS_InputMix) m_lpPS_InputMix->Release();
    GetDevice()->CreatePixelShader((DWORD*)pShaderByteCode->GetBufferPointer(), &m_lpPS_InputMix);
    if (pShaderByteCode) pShaderByteCode->Release();
    if (milkwave) milkwave->LogInfo(L"Input Mix Shader compiled successfully");
    // AddNotification(L"Input Mix Shader: READY", 1.0f);
  } else {
    // AddNotification(L"Input Mix Shader FAILED", 3.0f);
    if (pErrors) {
      char* err = (char*)pErrors->GetBufferPointer();
      if (err && milkwave) {
        milkwave->LogInfo(L"Shader Error:");
        wchar_t werr[1024];
        MultiByteToWideChar(CP_ACP, 0, err, -1, werr, 1024);
        milkwave->LogInfo(werr);
      }
      pErrors->Release();
    }
  }
}
