//////////////////////////////////////////////////////////////////////////////////
//
//  go_to_start_fork.S 
//
//  Copyright (C) 2004 by Jonas Maebe, Ghent University, Belgium
//
//  This file is part of DIOTA (http://www.elis.UGent.be/diota/)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
//
//////////////////////////////////////////////////////////////////////////////////
//
// $Author: nieblifn $
// $Date: 2004/08/31 17:02:14 $
// $Log: go_to_start_fork.S,v $
// Revision 1.1  2004/08/31 17:02:14  nieblifn
// *** empty log message ***
//
// Revision 1.1  2004/06/24 12:21:18  nieblifn
// *** empty log message ***
//
// Revision 1.3  2004/06/17 15:47:12  nieblifn
// *** empty log message ***
//
// Revision 1.2  2004/06/16 14:33:24  nieblifn
// *** empty log message ***
//
// Revision 1.1  2004/06/11 11:57:21  nieblifn
// *** empty log message ***
//
// Revision 1.1  2004/05/24 13:43:31  nieblifn
// *** empty log message ***
//
// Revision 1.2  2004/05/10 14:22:56  nieblifn
// clone & exec funktioniert mit execimage, test/fs/fs, und busybox.uclibc
//
// Revision 1.1  2004/04/30 12:06:59  nieblifn
// *** empty log message ***
//
// Revision 1.2  2004/04/14 12:30:11  nieblifn
// *** empty log message ***
//
//
// Revision 1.1.2.3  2004/01/22 13:18:10  jmaebe
// * changed copy loops to "rep movsl" so there are no separate basic blocks
//   anymore
// - removed "xorl %ecx,%ecx", as it's already zero after the copy loop
// + added zeroing of esi and edi
//
// Revision 1.1.2.2  2004/01/22 12:56:07  ronsse
// *** empty log message ***
//
// Revision 1.1.2.1  2004/01/20 13:21:56  ronsse
// + Support for statically linked binaries.
//
//
//////////////////////////////////////////////////////////////////////////////////

.globl go_to_start_fork
  .type go_to_start_fork,@function

go_to_start_fork:
   pushl %ebp
   movl  %esp, %ebp
   movl  8(%ebp), %eax  // address where new basepointer is stored
   movl  12(%ebp), %ebx // beginning of the stack
   movl  16(%ebp), %ecx // entrypoint
   movl  (%eax), %ebp
   addl  $28, %ebx      // empty the stack from the fork() call
   movl  %ebx, %esp

   xorl  %eax,%eax
   xorl  %ebx,%ebx
   xorl  %edx,%edx
   xorl  %esi,%esi
   xorl  %edi,%edi

   jmpl  *%ecx
   // should never be reached
   int3

  .size go_to_start_fork,.-go_to_start_fork

