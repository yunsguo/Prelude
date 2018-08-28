-- Functional parsing library from chapter 8 of Programming in
-- Haskell, Graham Hutton, Cambridge University Press, 2007.
-- 
-- Modified 29 March 2018 by Jim Royer to add Parser as an instance
-- of the Functor, Applicative, ... type classes to modernize the
-- code.
 
module Parsing where

import Data.Char
import Data.Functor
import qualified Control.Applicative as App (Applicative(..),Alternative(..))
import Control.Monad

-- The Parser type
------------------

newtype Parser a  =  P (String -> Maybe (a,String))

-- Joining up Parser to the Function, Applicative, Alternative, and
-- Monad, and MonadPlus type classes.

instance Functor Parser where
    fmap f p      =  P (\inp -> case parse p inp of
                                Nothing -> Nothing
                                Just (v,out) -> Just (f v,out))

instance App.Applicative Parser where
    pure          =  return
    (<*>)         =  ap
-- note: ap m1 m2 = do { x1 <- m1; x2 <- m2; return (x1 x2) }

instance App.Alternative Parser where
    empty         =  mzero
    (<|>)         =  mplus

instance Monad Parser where
   return v       =  P (\inp -> Just (v,inp))
   p >>= f        =  P (\inp -> case parse p inp of
                                  Nothing       -> Nothing
                                  Just (v,out)  -> parse (f v) out)

instance MonadPlus Parser where
   mzero          =  P (\inp -> Nothing)
   p `mplus` q    =  P (\inp -> case (parse p inp) of
                                  Nothing        -> parse q inp
                                  Just (v,out) -> Just (v,out))


-- basic parsers
----------------

failure           :: Parser a
failure           =  mzero

item              :: Parser Char
item              =  P (\inp -> case inp of
                                  ""     -> Nothing
                                  (x:xs) -> Just (x,xs))

parse             :: Parser a -> String -> Maybe (a,String)
parse (P p) inp   =  p inp

-- choice 
---------
(<|>)             :: Parser a -> Parser a -> Parser a
p <|> q           =  p `mplus` q
-- (note: hutton uses (***) instead of (<|>).) 


-- derived primitives
---------------------

sat               :: (Char -> Bool) -> Parser Char
sat p             =  do x <- item
                        if p x then return x else failure

digit, lower, upper, letter, alphanum  :: Parser Char

digit             =  sat isDigit
lower             =  sat isLower
upper             =  sat isUpper
letter            =  sat isAlpha
alphanum          =  sat isAlphaNum

char              :: Char -> Parser Char
char x            =  sat (== x)

string            :: String -> Parser String
string []         =  return []
string (x:xs)     =  do char x
                        string xs
                        return (x:xs)

many              :: Parser a -> Parser [a]
many p            =  many1 p <|> return []

many1             :: Parser a -> Parser [a]
many1 p           =  do v  <- p
                        vs <- many p
                        return (v:vs)

ident             :: Parser String
ident             =  do x  <- lower
                        xs <- many alphanum
                        return (x:xs)

nat               :: Parser Int
nat               =  do xs <- many1 digit
                        return (read xs)

int               :: Parser Int
int               =  do char '-'
                        n <- nat
                        return (-n)
                      <|> nat

space             :: Parser ()
space             =  do many (sat isSpace)
                        return ()

-- ignoring spacing
-------------------

token             :: Parser a -> Parser a
token p           =  do space
                        v <- p
                        space
                        return v

identifier        :: Parser String
identifier        =  token ident

natural           :: Parser Int
natural           =  token nat

integer           :: Parser Int
integer           =  token int

symbol            :: String -> Parser String
symbol xs         =  token (string xs)

-- Page 18 in Hutton's slides
dlist = do token(char '[')
           d <- token(digit)
           ds <- many (do {token(char ','); token(digit)})
           token(char ']')
           return (d:ds)

-- From pages 21 to 25 of Hutton's slides

-- expr   ::= term ('+' expr | epsilon )
-- term   ::= factor ('*' term | epsilon )
-- factor ::= dight | '(' expr ')'
-- digit  ::= '0' | '1' | ... | '9'

expr, term, factor :: Parser Int
                      
expr = do { t <- term
          ; do { char '+'
               ; e <- expr
               ; return (t + e)
               }
          <|> return t
          }

term = do { f <- factor
          ; do { char '*'
               ; t <- term
               ; return (f * t)
               }
          <|> return f
          }
factor = do { d <- digit; return (digitToInt d) }
         <|>
         do { char '('; e <- expr; char ')'; return e }
                 

eval :: String -> Int
eval xs = case (parse expr xs) of
            Nothing      -> error "parse failed"
            Just (v,_) -> v

